#pragma once
#include <cstdint>
namespace fgvk {
// A focus interruption invalidates recorded inputs even if no Present happened
// while the application was in the background. Sent window messages supply the
// revision; polling supplies the current state. Never change the game's clock.
struct BridgeFocus {
 enum class Change { None, Suspended, Resumed };
 bool known=false,active=false;
 uint64_t revision=0,generation=0,cutoff=0;
 Change Observe(bool focused,uint64_t acquired,uint64_t lossRevision){
  if(known && focused==active && lossRevision==revision)return Change::None;
  known=true;active=focused;revision=lossRevision;cutoff=acquired;++generation;
  return focused?Change::Resumed:Change::Suspended;
 }
 bool Accepts(uint64_t frame,uint64_t inputFrame,uint64_t inputGeneration)const{
  return known && active && frame>cutoff && inputFrame==frame && inputGeneration==generation;
 }
 bool BindPreAcquire(uint64_t frame,uint64_t preparedAfterAcquire,uint64_t inputGeneration,
                     uint64_t& inputFrame,unsigned unboundCount,bool outstandingAcquire)const{
  // RDR2 can record the SR snapshot before acquiring its output image. Bind
  // only the sole unbound snapshot from this generation to the immediately
  // following acquire. Never carry ambiguous/old data through a focus gap.
  if(inputFrame || unboundCount!=1 || outstandingAcquire || !frame ||
     preparedAfterAcquire!=frame-1 || !Accepts(frame,frame,inputGeneration))return false;
  inputFrame=frame;return true;
 }
};
}
