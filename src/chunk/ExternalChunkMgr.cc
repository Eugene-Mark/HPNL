#include "HPNL/ChunkMgr.h"

ExternalChunkMgr::ExternalChunkMgr() : buffer_num(0), buffer_size(0), buffer_id(0) {}

ExternalChunkMgr::ExternalChunkMgr(int buffer_num_, uint64_t buffer_size_) : buffer_num(buffer_num_), buffer_size(buffer_size_), buffer_id(0) {
  for (int i = 0; i < buffer_num*2; i++) {
    auto ck = new Chunk();
    ck->buffer = std::malloc(buffer_size);
    ck->capacity = buffer_size;
    ck->buffer_id = this->get_id();
    ck->mr = nullptr;
    this->reclaim(ck->buffer_id, ck);
  }
}

ExternalChunkMgr::~ExternalChunkMgr() {
  for (auto buf : buf_map) {
    std::free(buf.second->buffer);
    delete buf.second;
    buf.second = nullptr;
  }
  buf_map.clear();
}

Chunk* ExternalChunkMgr::get(int id) {
  std::lock_guard<std::mutex> l(mtx);
  return buf_map[id];
}

void ExternalChunkMgr::reclaim(int mid, Chunk* ck) {
  std::lock_guard<std::mutex> l(mtx);
  if (!buf_map.count(mid))
    buf_map[mid] = ck;
  bufs.push_back(ck);
}

Chunk* ExternalChunkMgr::get() {
  std::lock_guard<std::mutex> l(mtx);
  if (bufs.empty())
    return nullptr;
  Chunk *ck = bufs.back();
  bufs.pop_back();
  return ck;
}

int ExternalChunkMgr::free_size() {
  std::lock_guard<std::mutex> l(mtx);
  return bufs.size();
}

uint32_t ExternalChunkMgr::get_id() {
  return buffer_id++;
}
