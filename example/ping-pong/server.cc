#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

#include <iostream>

#define MSG_SIZE 4096
#define BUFFER_SIZE (65536*2)
#define BUFFER_NUM 128

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() = default;
    ~ShutdownCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
    }
};

class RecvCallback : public Callback {
  public:
    explicit RecvCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~RecvCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      auto ck = bufMgr->get(mid);
      ck->size = MSG_SIZE;
      auto con = (Connection*)ck->con;
      con->send(ck);

      auto new_ck = bufMgr->get();
      con->log_used_chunk(new_ck);
      con->activate_recv_chunk(new_ck);
    }
  private:
    ChunkMgr *bufMgr;
};

class SendCallback : public Callback {
  public:
    explicit SendCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~SendCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      bufMgr->reclaim(mid, ck);
      auto con = (Connection*)ck->con;
      con->remove_used_chunk(ck);
    }
  private:
    ChunkMgr *bufMgr;
};

int main(int argc, char *argv[]) {

  auto server = new Server(1, 16);
  server->init();

  ChunkMgr *bufMgr = new ChunkPool(server, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);
  server->set_buf_mgr(bufMgr);

  auto recvCallback = new RecvCallback(bufMgr);
  auto sendCallback = new SendCallback(bufMgr);
  auto shutdownCallback = new ShutdownCallback();
  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(nullptr);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.2.106", "12345");
  server->wait();

  delete sendCallback;
  delete recvCallback;
  delete shutdownCallback;
  delete server;
  delete bufMgr;
  return 0;
}
