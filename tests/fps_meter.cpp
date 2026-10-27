#include "fps_meter.h"
#include <cmath>
#include <cstdio>
int main(){
  fgvk::FPSMeter m;int failed=0;uint64_t now=100;
  auto check=[&](bool ok){if(!ok)++failed;};
  m.Present(now);
  auto run=[&](unsigned multiplier,bool telemetry){for(int i=0;i<50;++i){now+=20;if(telemetry)m.AddDisplayed(multiplier);m.Present(now);}};
  run(1,true);check(m.outputValid && std::abs(m.realFPS-50)<0.01f && std::abs(m.outputFPS-50)<0.01f);
  run(3,true);check(m.outputValid && std::abs(m.realFPS-50)<0.01f && std::abs(m.outputFPS-150)<0.01f);
  run(2,true);check(m.outputValid && std::abs(m.outputFPS-100)<0.01f);
  run(0,false);check(m.outputValid && std::abs(m.outputFPS-100)<0.01f); // one dropped window keeps last value
  run(0,false);run(0,false);check(!m.outputValid && std::abs(m.realFPS-50)<0.01f); // sustained loss clears it
  now+=10000;check(m.Present(now) && m.realFPS==0 && !m.outputValid); // pause publishes cleared values, not stale FPS
  run(3,true);
  check(m.outputValid && std::abs(m.realFPS-50)<0.01f && std::abs(m.outputFPS-150)<0.01f);
  m.ResetDisplayed();check(!m.outputValid && m.outputFPS==0 && m.displayed==0 && m.readings==0);
  // The SDK may alternate empty/native batches and generated batches. Keep
  // the real counts across those state reports rather than resetting on each.
  for(unsigned frame=0;frame<100;++frame){m.AddDisplayed(frame%2?4:0);now+=20;m.Present(now);}
  check(m.outputValid && std::abs(m.realFPS-50)<.01f && std::abs(m.outputFPS-100)<.01f);
  for(unsigned frame=0;frame<50;++frame){m.AddDisplayed(1);now+=20;m.Present(now);}
  check(m.outputValid && std::abs(m.outputFPS-50)<.01f);
  std::printf("FPS sampling checks (native/generated/asynchronous/dropout/pause/reset): %d failures\n",failed);
  return failed?1:0;
}
