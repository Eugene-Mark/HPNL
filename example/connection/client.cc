#include "HPNL/Connection.h"
#include "HPNL/Client.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

#include <iostream>

#define MSG_SIZE 4096
#define BUFFER_SIZE 65536
#define BUFFER_NUM 128
#define MAX_WORKERS 10

uint64_t start, end = 0;

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class ShutdownCallback : public Callback {
  public:
    explicit ShutdownCallback(Client *_clt) : clt(_clt) {}
    ~ShutdownCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      clt->shutdown();
    }
  private:
    Client *clt;
};

class ConnectedCallback : public Callback {
  public:
    ConnectedCallback(Client *client_, ChunkMgr *bufMgr_) : client(client_), bufMgr(bufMgr_) {}
    ~ConnectedCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::cout << "connected." << std::endl;
      auto *con = (Connection*)param_1;
      //client->shutdown(con);
    }
  private:
    Client *client;
    ChunkMgr *bufMgr;
};

void connect() {
  auto client = new Client(1, 16);
  client->init();

  ChunkMgr *bufMgr = new ChunkPool(client, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);
  client->set_buf_mgr(bufMgr);

  auto connectedCallback = new ConnectedCallback(client, bufMgr);
  auto shutdownCallback = new ShutdownCallback(client);

  client->set_connected_callback(connectedCallback);
  client->set_shutdown_callback(shutdownCallback);

  client->start();
  for (int i = 0; i < 50; i++) {
    client->connect("172.168.2.106", "12345");
  }

  client->wait();

  delete shutdownCallback;
  delete connectedCallback;
  delete client;
  delete bufMgr;
}

int main(int argc, char *argv[]) {
  connect();  
  return 0;
}
