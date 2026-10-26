#pragma once
#include <windows.h>
#include <dxgi.h>
#include <vector>
#include <mutex>
#include <string>
#include <cstdio>
#include "log.h"

namespace fgvk::xess_pacing {
// Optional, bounded diagnostics. Records the SDK's native Present calls and
// DXGI's last available display statistics; it does not alter delivery timing.
class OutputTrace {
public:
 struct DeadlineRow {
  int64_t now{},original{},final{},timestamp{},batchStart{},summaryCount{},summaryDuration{},interval{};
  unsigned frame{},index{},total{},applied{},samples{},valid{};
  int64_t phaseOrigin{};unsigned rebased{};
 };
private:
 struct Row {
  int64_t begin{},end{},syncQpc{},sampleQpc{};
  uint64_t frame{};
  unsigned generated{},interval{},flags{},presentCount{},presentRefresh{},syncRefresh{};
  long presentResult{},statisticsResult{};
 };
 std::mutex mutex_;
 std::vector<Row> rows_;
 std::vector<DeadlineRow> deadlines_;
 int64_t frequency_{},warmStart_{},lastGenerated_{},start_{};
 unsigned warmFrames_{};
 bool enabled_{},complete_{};
 DXGI_FRAME_STATISTICS lastStatistics_{};
 HRESULT lastStatisticsResult_=E_PENDING;
 int64_t lastSampleQpc_{};
 void Flush() noexcept {
  complete_=true;
  try {
   wchar_t executable[MAX_PATH]{};
   if(!GetModuleFileNameW(nullptr,executable,MAX_PATH))return;
   std::wstring path(executable);auto slash=path.find_last_of(L"\\/");
   if(slash==path.npos)return;
   path.resize(slash+1);
   path+=L"RDR2VulkanFG-XeSS-present-"+std::to_wstring(GetCurrentProcessId())+L".csv";
   FILE* file=nullptr;
   if(_wfopen_s(&file,path.c_str(),L"w") || !file){Log("XeSS output trace: could not open CSV");return;}
   fprintf(file,"QpcFrequency,BeginQpc,EndQpc,FrameId,Generated,SyncInterval,PresentFlags,PresentResult,StatisticsResult,PresentCount,PresentRefreshCount,SyncRefreshCount,SyncQpc,StatisticsSampleQpc\n");
   for(const auto& r:rows_)fprintf(file,"%lld,%lld,%lld,%llu,%u,%u,%u,%ld,%ld,%u,%u,%u,%lld,%lld\n",
    (long long)frequency_,(long long)r.begin,(long long)r.end,(unsigned long long)r.frame,
    r.generated,r.interval,r.flags,r.presentResult,r.statisticsResult,r.presentCount,
    r.presentRefresh,r.syncRefresh,(long long)r.syncQpc,(long long)r.sampleQpc);
   fclose(file);
   Log("XeSS output trace: saved %u native Present records to %S; DXGI statistics are delayed and HRESULT must be checked",(unsigned)rows_.size(),path.c_str());
   path.resize(slash+1);
   path+=L"RDR2VulkanFG-XeSS-deadline-"+std::to_wstring(GetCurrentProcessId())+L".csv";
   file=nullptr;
   if(_wfopen_s(&file,path.c_str(),L"w") || !file)return;
   fprintf(file,"NowNs,OriginalDeadlineNs,FinalDeadlineNs,TimestampNs,BatchStartNs,SummaryCount,SummaryDurationNs,IntervalNs,Frame,Index,Total,Applied,Samples,ValidMask,PhaseOriginNs,Rebased\n");
   for(const auto& r:deadlines_)fprintf(file,"%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%u,%u,%u,%u,%u,%u,%lld,%u\n",
    (long long)r.now,(long long)r.original,(long long)r.final,(long long)r.timestamp,(long long)r.batchStart,
    (long long)r.summaryCount,(long long)r.summaryDuration,(long long)r.interval,r.frame,r.index,r.total,r.applied,r.samples,r.valid,(long long)r.phaseOrigin,r.rebased);
   fclose(file);
   Log("XeSS output trace: saved %u owned deadline records to %S",(unsigned)deadlines_.size(),path.c_str());
  }catch(...){}
 }
public:
 void Enable(bool enabled) {
  enabled_=enabled;
  if(enabled){LARGE_INTEGER f{};if(QueryPerformanceFrequency(&f))frequency_=f.QuadPart;
   rows_.reserve(24000);deadlines_.reserve(24000);Log("XeSS output trace: armed; waits for stable generated output, then captures 20 seconds without ETW");}
 }
 static int64_t Tick() noexcept {LARGE_INTEGER q{};return QueryPerformanceCounter(&q)?q.QuadPart:0;}
 void RecordDeadline(const DeadlineRow& row) noexcept {
  if(!enabled_)return;
  try {std::lock_guard<std::mutex> lock(mutex_);
   if(start_ && !complete_ && deadlines_.size()<24000)deadlines_.push_back(row);
  }catch(...){}
 }
 // Called by the owning host thread using the SDK's public swapchain pointer.
 // Query outside our mutex to avoid holding a lock across an SDK/COM call.
 void Sample(IDXGISwapChain* swapchain) noexcept {
  if(!swapchain)return;
  try {
   {std::lock_guard<std::mutex> lock(mutex_);if(!enabled_ || complete_)return;}
   DXGI_FRAME_STATISTICS statistics{};
   const HRESULT result=swapchain->GetFrameStatistics(&statistics);
   const auto sampled=Tick();
   std::lock_guard<std::mutex> lock(mutex_);
   if(!complete_){lastStatistics_=statistics;lastStatisticsResult_=result;lastSampleQpc_=sampled;}
  }catch(...){}
 }
 void Record(uint64_t frame,bool generated,UINT interval,UINT flags,
             HRESULT result,int64_t begin,int64_t end) noexcept {
  if(!enabled_ || frequency_<=0 || begin<=0 || end<begin)return;
  try {
   std::lock_guard<std::mutex> lock(mutex_);
   if(complete_)return;
   if(!start_){
    if(!generated)return;
    if(!lastGenerated_ || begin-lastGenerated_>frequency_/4){warmStart_=begin;warmFrames_=0;}
    lastGenerated_=begin;++warmFrames_;
    if(warmFrames_<120 || begin-warmStart_<frequency_*2)return;
    start_=begin;Log("XeSS output trace: stable generated output; capture started");
   }
   const auto& statistics=lastStatistics_;
   rows_.push_back({begin,end,statistics.SyncQPCTime.QuadPart,lastSampleQpc_,frame,unsigned(generated),
    interval,flags,statistics.PresentCount,statistics.PresentRefreshCount,
    statistics.SyncRefreshCount,long(result),long(lastStatisticsResult_)});
   if(end-start_>=frequency_*20 || rows_.size()>=24000)Flush();
  }catch(...) {complete_=true;}
 }
};
}
