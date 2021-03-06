// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

package com.intel.hpnl.pingpong;

import java.nio.ByteBuffer;

import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Command;

import com.intel.hpnl.service.Client;
import com.intel.hpnl.core.Connection;

@Command(mixinStandardHelpOptions = true, version = "auto help demo - picocli 3.0")
public class ClientTest implements Runnable {

  @Option(names = {"-a", "--address"}, required = true, description = "server address")
  String addr = "localhost";

  @Option(names = {"-p", "--port"}, required = false, description = "server port")
  String port = "123456";

  @Option(names = {"-s", "--buffer_size"}, required = false, description = "buffer size")
  int bufferSize = 65536;

  @Option(names = {"-n", "--buffer_number"}, required = false, description = "buffer number")
  int bufferNbr = 32;

  @Option(names = {"-m", "--message_size"}, required = false, description = "pingpong message size")
  int msgSize = 4096;

  @Option(names = {"-w", "--worker_number"}, required = false, description = "worker numbers")
  int workNbr = 1;

  @Option(names = {"-i", "--interval"}, required = false, description = "statistics interval time")
  int interval = 5;

  @Option(names = {"-f", "--affinity"}, required = false, split = ",",  description = "HPNL thread affinity")
  int[] affinities = null;

  public void run() {
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(msgSize);
    for (int i = 0; i < msgSize; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    Client client = new Client(workNbr, bufferNbr);
    client.setAffinities(affinities);

    RecvCallback recvCallback = new RecvCallback(false, interval, msgSize);
    ShutdownCallback shutdownCallback = new ShutdownCallback();
    client.setRecvCallback(recvCallback);
    client.setShutdownCallback(shutdownCallback);

    client.initBufferPool(bufferNbr, bufferSize, bufferNbr);
    client.start();

    Connection con = client.connect(addr, port, 0);
    assert(con != null);
    System.out.println("connected, start to pingpong");
    con.send(byteBufferTmp, (byte)0, 10);

    client.join();
  }

  public static void main(String... args) {
    CommandLine.run(new ClientTest(), args);
  }
}
