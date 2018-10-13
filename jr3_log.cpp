// copied from nitta_shm.cpp by hiroki takeda
// read force sensor

#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <assert.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <stdexcept>
#include <jr3qlib.h>

//#if (ROBOT==2) // JAXON1 2port
// hot fix for JAXON_RED with jr3d
#define JR3Q_CHNUM 2
// #else
// #define JR3Q_CHNUM 4
// #endif

#define INTERVAL 500
#define BUF_SIZE 1000 * 3
float (*buf)[2][6];
timespec *buf_tm;

static timespec g_ts;
static int g_period_ns=1000000;
int count = 0;

bool flag = false;

void timespec_add_ns(timespec *ts, int ns)
{
    ts->tv_nsec += ns;
    while (ts->tv_nsec > 1e9){
        ts->tv_sec += 1;
        ts->tv_nsec -= 1e9;
    }
}

double timespec_compare(timespec *ts1, timespec *ts2)
{
    double dts = ts1->tv_sec - ts2->tv_sec;
    double dtn = ts1->tv_nsec - ts2->tv_nsec;
    return dts*1e9+dtn;
}

int wait_for_period()
{
    //std::cerr << "dummy IOB wait (art)" << std::endl;
#ifndef USE_ART
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &g_ts, 0);
    timespec_add_ns(&g_ts, g_period_ns);
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double dt = timespec_compare(&g_ts, &now);
    if (dt <= 0){
        //printf("overrun(%d[ms])\n", -dt*1e6);
        do {
            timespec_add_ns(&g_ts, g_period_ns);
        }while(timespec_compare(&g_ts, &now)<=0);
    }
#endif
    return 0;
}

void handler(int signum) {
  flag = true;
}

int main() {
  try {
    IJR3Q* ptest_dev = JR3QFactory::create( JR3Q_TYPE_AUTO, JR3Q_CHNUM );

	buf = new float[BUF_SIZE][2][6];
	buf_tm = new timespec[BUF_SIZE];
	
	// reset offsets
	for( int ch = 0; ch < JR3Q_CHNUM; ch++ ) {
      float f0[6];
      ptest_dev->getCh( ch )->read( f0 );
      printf( "%d: ", ch );
      for( int i = 0; i < 6; i++ )
        printf( i<3?"%9.5f[N], ":"%9.5f[Nm], ", f0[i] );
      printf( "\n");
    }
    printf( "reset offsets\n");
    sleep(1);
    for( int ch = 0; ch < JR3Q_CHNUM; ch++ )
      ptest_dev->getCh( ch )->reset_offsets();
    sleep(1);

	// signal handler
	if(signal(SIGINT, handler) == SIG_ERR) {}

    while(!flag) {
	  
	  // read JR3 sensor value and check its value is not all zero
      float f0[JR3Q_CHNUM][6];
      bool is_all_zero = true;
      for( int ch = 0; ch < JR3Q_CHNUM; ch++ ) {
        ptest_dev->getCh( ch )->read( f0[ch] );
        for (int i = 0; i < 6; i++)
          if (f0[ch][i] != 0.0)
            is_all_zero = false;
	  }

	  if(count>((int)(BUF_SIZE))) printf("\x1b[34m");

      if (!is_all_zero) {

		clock_gettime(CLOCK_MONOTONIC, &buf_tm[count%((int)(BUF_SIZE))]);

        for( int ch = 0; ch < JR3Q_CHNUM; ch++ ) {
          float f0[6];
          ptest_dev->getCh( ch )->read( f0 );
          if(count%((int)(INTERVAL))==0) printf( "%d: ", ch );
          for( int i = 0; i < 6; i++ ){
			buf[count%((int)(BUF_SIZE))][ch][i] = f0[i];
            if(count%((int)(INTERVAL))==0)
              printf( i<3?"%9.5f[N], ":"%9.5f[Nm], ", f0[i] );
		  }
          if(count%((int)(INTERVAL))==0) printf( "\n" );
        }
      } else {
        printf( "\x1b[31mJR3 values are all zero. values will not be updated.\n\x1b[0m");
      }

	  printf("\x1b[0m");
      wait_for_period();
      count++;
    }
	
	printf("save log\n");
	time_t today = time(NULL);
	char filename[100];
	strftime(filename, sizeof(filename), "log/jr3%y%m%d_%H%M%S.log", localtime(&today));
	FILE *fp;
	if ((fp = fopen(filename, "w")) == NULL)
	  printf("cannot open file\n");
	int cnt=0;
	while(cnt<BUF_SIZE){
	  fprintf(fp, "%ld.%09ld ", buf_tm[(count+cnt)%((int)(BUF_SIZE))].tv_sec, buf_tm[(count+cnt)%((int)(BUF_SIZE))].tv_nsec);
	  for (int ch=0;ch<2;ch++)
		for (int i=0;i<6;i++)
		  fprintf(fp, "%f ", buf[(count+cnt)%((int)(BUF_SIZE))][ch][i]);
	  fprintf(fp, "\n");
	  cnt++;
	}
  }
  catch( const std::runtime_error &e ){
    printf( "error: %s\n", e.what() );
  }

  return 0;
}
