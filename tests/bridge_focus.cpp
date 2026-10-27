#include "../src/bridge_focus.h"
#include <cassert>
#include <cstdio>
int main(){
 using fgvk::BridgeFocus;using Change=BridgeFocus::Change;
 BridgeFocus focus;
 assert(!focus.Accepts(1,1,0));
 assert(focus.Observe(true,0,0)==Change::Resumed);
 auto generation=focus.generation;
 assert(focus.Accepts(1,1,generation));
 assert(focus.Observe(true,1,0)==Change::None && focus.Accepts(2,2,generation));
 assert(!focus.Accepts(2,1,generation) && !focus.Accepts(2,0,generation));
 assert(focus.Observe(false,2,1)==Change::Suspended);
 assert(!focus.Accepts(3,3,focus.generation));
 assert(focus.Observe(false,3,1)==Change::None);
 assert(focus.Observe(true,3,1)==Change::Resumed);
 assert(!focus.Accepts(3,3,focus.generation));
 assert(!focus.Accepts(4,4,generation));
 assert(focus.Accepts(4,4,focus.generation));
 // No acquire/present while tabbed out: sent messages still force fresh data.
 assert(focus.Observe(true,4,2)==Change::Resumed);
 assert(!focus.Accepts(4,4,focus.generation));
 assert(focus.Accepts(5,5,focus.generation));
 // Actual ordering supported by the transport: SR record -> output acquire
 // -> SR submit -> present. An unbound epoch must acquire a proven fresh ID
 // before the strict presentation check, rather than weakening Accepts().
 uint64_t preAcquire=0;
 assert(!focus.Accepts(5,preAcquire,focus.generation));
 assert(focus.BindPreAcquire(5,4,focus.generation,preAcquire,1,false));
 assert(preAcquire==5 && focus.Accepts(5,preAcquire,focus.generation));
 assert(!focus.BindPreAcquire(6,5,focus.generation,preAcquire,1,false));
 auto rejected=[&](uint64_t frame,uint64_t prepared,uint64_t gen,unsigned count,bool outstanding){
  uint64_t unbound=0;const bool accepted=focus.BindPreAcquire(frame,prepared,gen,unbound,count,outstanding);
  assert(!accepted && !unbound);
 };
 rejected(5,4,focus.generation,2,false); // multiple pending snapshots
 rejected(5,4,focus.generation,1,true); // another image is already acquired
 rejected(6,4,focus.generation,1,false); // stale, skipped acquire
 rejected(4,3,focus.generation,1,false); // focus cutoff
 rejected(5,4,generation,1,false); // snapshot from an earlier focus generation
 assert(focus.Observe(false,5,3)==Change::Suspended);
 rejected(6,5,focus.generation,1,false); // background data
 assert(focus.Observe(true,5,3)==Change::Resumed);
 for(unsigned i=0;i<1000;++i){
  const auto epoch=uint64_t(i)*3+6,old=focus.generation;
  assert(focus.Observe(false,epoch,i+4)==Change::Suspended);
  assert(!focus.Accepts(epoch+1,epoch+1,focus.generation));
  assert(focus.Observe(true,epoch+1,i+4)==Change::Resumed);
  assert(!focus.Accepts(epoch+2,epoch+2,old));
  assert(focus.Accepts(epoch+2,epoch+2,focus.generation));
 }
 puts("Focus recovery: pre-acquire SR association, ambiguity/stale/background rejection, 1000 focus cycles: PASS");
}
