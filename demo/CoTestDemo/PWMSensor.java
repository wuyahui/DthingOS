import java.io.DataInputStream;
import java.io.IOException;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;

import iot.oem.pwm.PWMManager;

public class PWMSensor {
	  private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
	  private boolean allowLogOnServer = true;
	
	  public PWMSensor() {
	    // TODO Auto-generated constructor stub
	  }
	
  	public void startTest() {
      
        int i = 0;
        int  hz = 1;
        System.out.println("[PWMDemo]Start PWM sensor test");
        PWMManager manager = PWMManager.getInstance();
        
        try {
		        manager.command(0, 0x30, 1);
		        manager.command(0, 0x31, 1);
        
		        for (i = 0; i < 10; i++)
		        {
		              manager.config(0, hz, i*10);
		    
		              if (hz < 5)
		              {
		                hz++;
		              }
		              else
		              {
		                hz = 1;
		              }
		        }
		        System.out.println("[PWMDemo]Test success");
		        reportPWMInfo("Test success");
		        manager.command(0, 0x30, 0);
        } catch (IOException e) {
            System.out.println("[PWMDemo] exception happened!");
              try {
                  reportPWMInfo("IOException");
                  manager.command(0, 0x30, 0);
              } catch (IOException e1) {
                  e1.printStackTrace();
              }
              e.printStackTrace();
        }
        
        System.out.println("[PWMDemo] End PWM sensor test");
    }

    private void reportPWMInfo(String msg) throws IOException
    {
        String content = "PWM:" + msg;
        String reportInfo = REPORT_SERVER_FORMAT + content;
    
        if (!allowLogOnServer)
        {
            System.out.println("[PWMSensor]" + content);
            return;
        }
    
        HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
        httpConn.setRequestMethod(HttpConnection.POST);
        DataInputStream dis = httpConn.openDataInputStream();
        dis.close();
        httpConn.close();
    }
}
