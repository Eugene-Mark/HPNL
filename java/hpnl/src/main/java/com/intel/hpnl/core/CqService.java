package com.intel.hpnl.core;

import java.util.List;
import java.util.ArrayList;

public class CqService {
  static {
    System.load("/usr/local/lib/libhpnl.so");
  }

  public CqService(EqService service, long serviceNativeHandle) {
    this.eqService = service;
    this.serviceNativeHandle = serviceNativeHandle;

    //Runtime.getRuntime().addShutdownHook(new CqShutdownThread(eqService, this));
    this.cqThreads = new ArrayList<CqThread>();
    this.externalHandlers = new ArrayList<ExternalHandler>();
  }

  public CqService init() {
    if (init(serviceNativeHandle) == -1)
      return null;
    return this;
  }

  public int start() {
    int workerNum = this.eqService.getWorkerNum();
    for (int i = 0; i < workerNum; i++) {
      CqThread cqThread = new CqThread(this, i);
      cqThreads.add(cqThread);
    }
    for (CqThread cqThread : cqThreads) {
      cqThread.start();
    }
    return 0;
  }

  public void shutdown() {
    for (CqThread cqThread : cqThreads) {
      synchronized(this) {
        cqThread.shutdown();
      }
    }
  }

  public void join() {
    try {
      for (CqThread cqThread : cqThreads) {
        cqThread.join();
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
      synchronized(this) {
        free();
      }
    }
  }

  private void handleCqCallback(long eq, int eventType, int rdmaBufferId, int block_buffer_size) {
    Connection connection = eqService.getCon(eq);
    if (connection != null) {
      connection.handleCallback(eventType, rdmaBufferId, block_buffer_size);
    }
  }

  public void addExternalEvent(ExternalHandler externalHandler) {
    externalHandlers.add(externalHandler); 
  }

  private int waitExternalEvent(int index) {
    for (ExternalHandler handler: externalHandlers) {
      handler.handle();
    }
    return 0;
  }

  public int wait_event(int index) {
    if (wait_cq_event(index) < 0) {
      return -1;
    }
    waitExternalEvent(index);
    return 0;
  }

  public native int wait_cq_event(int index);
  private native int init(long Service);
  public native void finalize();
  private native void free();
  private long nativeHandle;
  private EqService eqService;
  private long serviceNativeHandle;
  private List<CqThread> cqThreads;
  private List<ExternalHandler> externalHandlers;
}
