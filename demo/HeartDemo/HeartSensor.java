/**
 * Copyright 2012-2015 Yarlung Soft. All Rights Reserved.
 * $Id: $
 */
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.adc.ADCManager;

/**
 * 心跳传感器演示软件
 *
 * 测试次数：96000 采样间隔：50毫秒  端 口: 0
 */
public class HeartSensor extends Applet {
    private static final int DEFAULT_ADC_CHANNEL = 0;
    private static final int MAX_TEST_COUNT = 96000;
    private static final int MAX_BUFFER_COUNT = 1200;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=csxl&parmInfo=";
    private static int count = MAX_TEST_COUNT;
    private static int reportDataIndex = 0;
    private static int readDataCount = 0;
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;
    private static int[] buffer = new int[MAX_BUFFER_COUNT];
    private static long totalMemory = 0;
    private static long gcMemory = 0;

    public HeartSensor() {
        // TODO Auto-generated constructor stub
    }

    protected static void resetBuffer()
    {
        readDataCount = 0;
        reportDataIndex = 0;
        buffer = new int[MAX_BUFFER_COUNT];
    }

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        totalMemory = Runtime.getRuntime().totalMemory();
        gcMemory = totalMemory * 3 / 10;

        //ADC采样线程
        new Thread() {
            private long lastScheduledTime = 0;
            private ADCManager manager = null;

            public void run() {
                do {
                    long currentTime = System.currentTimeMillis();
                    if ((currentTime - lastScheduledTime) >= 50)
                    {
                        lastScheduledTime = currentTime;
                        read();
                    }
                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException e) {
                        System.out.println(e.getMessage());
                    }
                } while ((count > 0) && allowRunning);
                notifyDestroyed();
            }

            public void read() {
                System.out.println("Starting Heart sensor read task ...");

                // 创建心跳传感器实例
                manager = ADCManager.getInstance();

                if (manager == null)
                    return;

                if (count <= 0)
                {
                    // 销毁心跳传感器实例
                    try {
                        manager.destroy();
                    } catch (IOException e1) {
                        System.out.println("Heart IOException: " + e1);
                    }
                }

                // 读取心跳数据并缓存
                try {
                    if (readDataCount < MAX_BUFFER_COUNT)
                    {
                        buffer[readDataCount] = manager.read(DEFAULT_ADC_CHANNEL);
                        readDataCount++;
                        count--;
                    }
                } catch (IllegalArgumentException e) {
                    System.out.println("Heart IllegalArgumentException:" + e);
                } catch (IOException e) {
                    System.out.println("Heart IOException:" + e);
                }

                System.out.println("Exiting Heart sensor read task ...");
            }
        }.start();

        new Thread() {
            public void run() {
                System.out.println("Starting report adc data ...");

                do {
                    if ((readDataCount == MAX_BUFFER_COUNT)
                        && (reportDataIndex == readDataCount))
                    {
                        HeartSensor.resetBuffer();
                    }

                    if (reportDataIndex < readDataCount)
                    {
                        try {
                            int temp = buffer[reportDataIndex] * 35;
                            int head = temp / 10000;
                            String tail = "0000" + (temp % 10000);
                            int len = tail.length();
                            String value = head + "." + tail.substring(len - 4, len);
                            reportDataIndex++;
                            reportADCInfo(value, DEFAULT_ADC_CHANNEL);
                        } catch (IOException e) {
                            System.out.println("Heart IOException:" + e);
                        }
                    }
                    try {
                        MemoryCheck();
                        Thread.sleep(1);
                    } catch (InterruptedException e) {
                        System.out.println(e.getMessage());
                    }
                } while ((count > 0) && allowRunning);
                notifyDestroyed();
            }
            /**
             * @brief 上报心跳信息到指定服务器
             * @param value 端口数据
             * @param cid 端口ID
             * @exception 当网络连接有问题时抛出IO异常
             */
            private void reportADCInfo(String value, int cid) throws IOException {
                String content = "Heart:" + value + " CID:" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + value;

                if (allowLogPrint) {
                    System.out.println("[Heart][Test Result]" + content);
                }

                URL url = new URL(reportInfo);
                HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
                httpConn.setRequestMethod(HttpURLConnection.POST);
                InputStream dis = httpConn.getInputStream();
                dis.close();
                httpConn.disconnect();
            }
        }.start();
    }

    public static void MemoryCheck() {
        long free =  Runtime.getRuntime().freeMemory();

        //Force VM to gc when memory is less than 30%
        if (free < gcMemory) {
            Runtime.getRuntime().gc();
        }
    }
}