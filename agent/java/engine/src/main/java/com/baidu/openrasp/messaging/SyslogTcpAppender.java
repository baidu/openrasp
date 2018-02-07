/*
 * Copyright 2017-2018 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


package com.baidu.openrasp.messaging;

import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.net.SocketNode;
import org.apache.log4j.net.ZeroConfSupport;
import org.apache.log4j.spi.ErrorCode;
import org.apache.log4j.spi.LoggingEvent;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class SyslogTcpAppender extends AppenderSkeleton {

  public static String LINE_SEP = System.getProperty("line.separator");

  /** Kernel messages */
  final static public int LOG_KERN     = 0;
  /** Random user-level messages */
  final static public int LOG_USER     = 1<<3;
  /** Mail system */
  final static public int LOG_MAIL     = 2<<3;
  /** System daemons */
  final static public int LOG_DAEMON   = 3<<3;
  /** security/authorization messages */
  final static public int LOG_AUTH     = 4<<3;
  /** messages generated internally by syslogd */
  final static public int LOG_SYSLOG   = 5<<3;

  /** line printer subsystem */
  final static public int LOG_LPR      = 6<<3;
  /** network news subsystem */
  final static public int LOG_NEWS     = 7<<3;
  /** UUCP subsystem */
  final static public int LOG_UUCP     = 8<<3;
  /** clock daemon */
  final static public int LOG_CRON     = 9<<3;
  /** security/authorization  messages (private) */
  final static public int LOG_AUTHPRIV = 10<<3;
  /** ftp daemon */
  final static public int LOG_FTP      = 11<<3;

  // other codes through 15 reserved for system use
  /** reserved for local use */
  final static public int LOG_LOCAL0 = 16<<3;
  /** reserved for local use */
  final static public int LOG_LOCAL1 = 17<<3;
  /** reserved for local use */
  final static public int LOG_LOCAL2 = 18<<3;
  /** reserved for local use */
  final static public int LOG_LOCAL3 = 19<<3;
  /** reserved for local use */
  final static public int LOG_LOCAL4 = 20<<3;
  /** reserved for local use */
  final static public int LOG_LOCAL5 = 21<<3;
  /** reserved for local use */
  final static public int LOG_LOCAL6 = 22<<3;
  /** reserved for local use*/
  final static public int LOG_LOCAL7 = 23<<3;

  protected static final int SYSLOG_HOST_OI = 0;
  protected static final int FACILITY_OI = 1;

  static final String TAB = "    ";

  // Have LOG_USER as default
  int syslogFacility = LOG_USER;
  String facilityStr;
  boolean facilityPrinting = false;
  SyslogTcpWriter stw;

  /**
   * If true, the appender will generate the HEADER (timestamp and host name)
   * part of the syslog packet.
   * @since 1.2.15
   */
  private boolean header = false;
  /**
   * Date format used if header = true.
   * @since 1.2.15
   */
  private final SimpleDateFormat dateFormat = new SimpleDateFormat("MMM dd HH:mm:ss ", Locale.ENGLISH);
  /**
   * Host name used to identify messages from this appender.
   * @since 1.2.15
   */
  private String localHostname;

  /**
     The default port number of remote logging server (4560).
     @since 1.2.15
  */
  static public final int DEFAULT_PORT                 = 514;

  /**
     The default reconnection delay (30000 milliseconds or 30 seconds).
  */
  static final int DEFAULT_RECONNECTION_DELAY   = 30000;

  /**
     We remember host name as String in addition to the resolved
     InetAddress so that it can be returned via getOption().
  */
  String remoteHost;

  /**
   * The MulticastDNS zone advertised by a SocketAppender
   */
  public static final String ZONE = "_log4j_obj_tcpconnect_appender.local.";

  InetAddress address;
  int port = DEFAULT_PORT;
  int reconnectionDelay = DEFAULT_RECONNECTION_DELAY;

  private Connector connector;
  private boolean advertiseViaMulticastDNS;
  private ZeroConfSupport zeroConf;

  public SyslogTcpAppender() {
  }

  /**
     Connects to remote server at <code>address</code> and <code>port</code>.
  */
  public SyslogTcpAppender(InetAddress address, int port, int syslogFacility) {
    this.address = address;
    this.remoteHost = address.getHostName();
    this.port = port;
    this.syslogFacility = syslogFacility;
    this.initSyslogFacilityStr();
    connect(address, port);
  }

  /**
     Connects to remote server at <code>host</code> and <code>port</code>.
  */
  public SyslogTcpAppender(String host, int port, int syslogFacility) {
    this.port = port;
    this.address = getAddressByName(host);
    this.remoteHost = host;
    this.syslogFacility = syslogFacility;
    this.initSyslogFacilityStr();
    connect(address, port);
  }

  /**
     Connect to the specified <b>RemoteHost</b> and <b>Port</b>.
  */
  public void activateOptions() {
    if (advertiseViaMulticastDNS) {
      zeroConf = new ZeroConfSupport(ZONE, port, getName());
      zeroConf.advertise();
    }
    connect(address, port);
  }

  /**
   * Close this appender.  
   *
   * <p>This will mark the appender as closed and call then {@link
   * #cleanUp} method.
   * */
  synchronized public void close() {
    if(closed)
      return;

    this.closed = true;
    if (advertiseViaMulticastDNS) {
      zeroConf.unadvertise();
    }

    cleanUp();
  }

  private
  void initSyslogFacilityStr() {
    facilityStr = getFacilityString(this.syslogFacility);

    if (facilityStr == null) {
      System.err.println("\"" + syslogFacility +
              "\" is an unknown syslog facility. Defaulting to \"USER\".");
      this.syslogFacility = LOG_USER;
      facilityStr = "user:";
    } else {
      facilityStr += ":";
    }
  }

  /**
   Returns the specified syslog facility as a lower-case String,
   e.g. "kern", "user", etc.
   */
  public
  static
  String getFacilityString(int syslogFacility) {
    switch(syslogFacility) {
      case LOG_KERN:      return "kern";
      case LOG_USER:      return "user";
      case LOG_MAIL:      return "mail";
      case LOG_DAEMON:    return "daemon";
      case LOG_AUTH:      return "auth";
      case LOG_SYSLOG:    return "syslog";
      case LOG_LPR:       return "lpr";
      case LOG_NEWS:      return "news";
      case LOG_UUCP:      return "uucp";
      case LOG_CRON:      return "cron";
      case LOG_AUTHPRIV:  return "authpriv";
      case LOG_FTP:       return "ftp";
      case LOG_LOCAL0:    return "local0";
      case LOG_LOCAL1:    return "local1";
      case LOG_LOCAL2:    return "local2";
      case LOG_LOCAL3:    return "local3";
      case LOG_LOCAL4:    return "local4";
      case LOG_LOCAL5:    return "local5";
      case LOG_LOCAL6:    return "local6";
      case LOG_LOCAL7:    return "local7";
      default:            return null;
    }
  }

  /**
   Returns the integer value corresponding to the named syslog
   facility, or -1 if it couldn't be recognized.

   @param facilityName one of the strings KERN, USER, MAIL, DAEMON,
   AUTH, SYSLOG, LPR, NEWS, UUCP, CRON, AUTHPRIV, FTP, LOCAL0,
   LOCAL1, LOCAL2, LOCAL3, LOCAL4, LOCAL5, LOCAL6, LOCAL7.
   The matching is case-insensitive.

   @since 1.1
   */
  public
  static
  int getFacility(String facilityName) {
    if(facilityName != null) {
      facilityName = facilityName.trim();
    }
    if("KERN".equalsIgnoreCase(facilityName)) {
      return LOG_KERN;
    } else if("USER".equalsIgnoreCase(facilityName)) {
      return LOG_USER;
    } else if("MAIL".equalsIgnoreCase(facilityName)) {
      return LOG_MAIL;
    } else if("DAEMON".equalsIgnoreCase(facilityName)) {
      return LOG_DAEMON;
    } else if("AUTH".equalsIgnoreCase(facilityName)) {
      return LOG_AUTH;
    } else if("SYSLOG".equalsIgnoreCase(facilityName)) {
      return LOG_SYSLOG;
    } else if("LPR".equalsIgnoreCase(facilityName)) {
      return LOG_LPR;
    } else if("NEWS".equalsIgnoreCase(facilityName)) {
      return LOG_NEWS;
    } else if("UUCP".equalsIgnoreCase(facilityName)) {
      return LOG_UUCP;
    } else if("CRON".equalsIgnoreCase(facilityName)) {
      return LOG_CRON;
    } else if("AUTHPRIV".equalsIgnoreCase(facilityName)) {
      return LOG_AUTHPRIV;
    } else if("FTP".equalsIgnoreCase(facilityName)) {
      return LOG_FTP;
    } else if("LOCAL0".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL0;
    } else if("LOCAL1".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL1;
    } else if("LOCAL2".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL2;
    } else if("LOCAL3".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL3;
    } else if("LOCAL4".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL4;
    } else if("LOCAL5".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL5;
    } else if("LOCAL6".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL6;
    } else if("LOCAL7".equalsIgnoreCase(facilityName)) {
      return LOG_LOCAL7;
    } else {
      return -1;
    }
  }

  /**
   * Drop the connection to the remote host and release the underlying
   * connector thread if it has been created 
   * */
  public void cleanUp() {
    if(stw != null) {
      try {
	stw.close();
      } catch(IOException e) {
          if (e instanceof InterruptedIOException) {
              Thread.currentThread().interrupt();
          }
	      LogLog.error("Could not close stw.", e);
      }
      stw = null;
    }
    if(connector != null) {
      //LogLog.debug("Interrupting the connector.");
      connector.interrupted = true;
      connector = null;  // allow gc
    }
  }

  void connect(InetAddress address, int port) {
    if(this.address == null)
      return;
    try {
      // First, close the previous connection if any.
      cleanUp();
      stw = new SyslogTcpWriter(new Socket(address, port).getOutputStream(), syslogFacility);
    } catch(IOException e) {
      if (e instanceof InterruptedIOException) {
          Thread.currentThread().interrupt();
      }
      String msg = "Could not connect to remote log4j server at ["
	+address.getHostName()+"].";
      if(reconnectionDelay > 0) {
        msg += " We will try again later.";
	fireConnector(); // fire the connector thread
      } else {
          msg += " We are not retrying.";
          errorHandler.error(msg, e, ErrorCode.GENERIC_FAILURE);
      } 
      LogLog.error(msg);
    }
  }


  public void append(LoggingEvent event) {
    if(event == null)
      return;

    if(!isAsSevereAsThreshold(event.getLevel()))
      return;

    if(address==null) {
      errorHandler.error("No remote host is set for SyslogTcpAppender named \""+
			this.name+"\".");
      return;
    }

    if(stw == null) {
      errorHandler.error("No syslog host is set for SyslogTcpAppender named \""+
              this.name+"\".");
      return;
    }

    if(stw != null) {
      try {
        String hdr = getPacketHeader(event.timeStamp);
        String packet;
        packet = String.valueOf(event.getMessage());
        if(facilityPrinting || hdr.length() > 0) {
          StringBuffer buf = new StringBuffer(hdr);
          if(facilityPrinting) {
            buf.append(facilityStr);
          }
          buf.append(packet);
          buf.append(LINE_SEP);
          packet = buf.toString();
        }
        stw.setLevel(event.getLevel().getSyslogEquivalent());
        stw.writeString(packet);
        stw.flush();
      } catch(IOException e) {
        if (e instanceof InterruptedIOException) {
          Thread.currentThread().interrupt();
        }
        stw = null;
        LogLog.warn("Detected problem with connection: "+e);
        if(reconnectionDelay > 0) {
          fireConnector();
        } else {
          errorHandler.error("Detected problem with connection, not reconnecting.", e,
                  ErrorCode.GENERIC_FAILURE);
        }
      }
    }

  }

  public void setAdvertiseViaMulticastDNS(boolean advertiseViaMulticastDNS) {
    this.advertiseViaMulticastDNS = advertiseViaMulticastDNS;
  }

  public boolean isAdvertiseViaMulticastDNS() {
    return advertiseViaMulticastDNS;
  }

  void fireConnector() {
    if(connector == null) {
      LogLog.debug("Starting a new connector thread.");
      connector = new Connector();
      connector.setDaemon(true);
      connector.setPriority(Thread.MIN_PRIORITY);
      connector.start();
    }
  }

  static
  InetAddress getAddressByName(String host) {
    try {
      return InetAddress.getByName(host);
    } catch(Exception e) {
      if (e instanceof InterruptedIOException || e instanceof InterruptedException) {
          Thread.currentThread().interrupt();
      }
      LogLog.error("Could not find address of ["+host+"].", e);
      return null;
    }
  }

  /**
   * The SocketAppender does not use a layout. Hence, this method
   * returns <code>false</code>.  
   * */
  public boolean requiresLayout() {
    return false;
  }

  /**
   * The <b>RemoteHost</b> option takes a string value which should be
   * the host name of the server where a {@link SocketNode} is
   * running.
   * */
  public void setRemoteHost(String host) {
    address = getAddressByName(host);
    remoteHost = host;
  }

  /**
     Returns value of the <b>RemoteHost</b> option.
   */
  public String getRemoteHost() {
    return remoteHost;
  }

  /**
     The <b>Port</b> option takes a positive integer representing
     the port where the server is waiting for connections.
   */
  public void setPort(int port) {
    this.port = port;
  }

  /**
     Returns value of the <b>Port</b> option.
   */
  public int getPort() {
    return port;
  }

  /**
     The <b>ReconnectionDelay</b> option takes a positive integer
     representing the number of milliseconds to wait between each
     failed connection attempt to the server. The default value of
     this option is 30000 which corresponds to 30 seconds.

     <p>Setting this option to zero turns off reconnection
     capability.
   */
  public void setReconnectionDelay(int delay) {
    this.reconnectionDelay = delay;
  }

  /**
     Returns value of the <b>ReconnectionDelay</b> option.
   */
  public int getReconnectionDelay() {
    return reconnectionDelay;
  }

  /**
   Set the syslog facility. This is the <b>Facility</b> option.

   <p>The <code>facilityName</code> parameter must be one of the
   strings KERN, USER, MAIL, DAEMON, AUTH, SYSLOG, LPR, NEWS, UUCP,
   CRON, AUTHPRIV, FTP, LOCAL0, LOCAL1, LOCAL2, LOCAL3, LOCAL4,
   LOCAL5, LOCAL6, LOCAL7. Case is unimportant.

   @since 0.8.1 */
  public
  void setFacility(String facilityName) {
    if(facilityName == null)
      return;

    syslogFacility = getFacility(facilityName);
    if (syslogFacility == -1) {
      System.err.println("["+facilityName +
              "] is an unknown syslog facility. Defaulting to [USER].");
      syslogFacility = LOG_USER;
    }

    this.initSyslogFacilityStr();

    // If there is already a stw, make it use the new facility.
    if(stw != null) {
      stw.setSyslogFacility(this.syslogFacility);
    }
  }

  /**
   Returns the value of the <b>Facility</b> option.
   */
  public
  String getFacility() {
    return getFacilityString(syslogFacility);
  }

  /**
   If the <b>FacilityPrinting</b> option is set to true, the printed
   message will include the facility name of the application. It is
   <em>false</em> by default.
   */
  public
  void setFacilityPrinting(boolean on) {
    facilityPrinting = on;
  }

  /**
   Returns the value of the <b>FacilityPrinting</b> option.
   */
  public
  boolean getFacilityPrinting() {
    return facilityPrinting;
  }

  /**
   * If true, the appender will generate the HEADER part (that is, timestamp and host name)
   * of the syslog packet.  Default value is false for compatibility with existing behavior,
   * however should be true unless there is a specific justification.
   * @since 1.2.15
   */
  public final boolean getHeader() {
    return header;
  }

  /**
   * Returns whether the appender produces the HEADER part (that is, timestamp and host name)
   * of the syslog packet.
   * @since 1.2.15
   */
  public final void setHeader(final boolean val) {
    header = val;
  }

  /**
   * Get the host name used to identify this appender.
   * @return local host name
   * @since 1.2.15
   */
  private String getLocalHostname() {
    if (localHostname == null) {
      try {
        InetAddress addr = InetAddress.getLocalHost();
        localHostname = addr.getHostName();
      } catch (UnknownHostException uhe) {
        localHostname = "UNKNOWN_HOST";
      }
    }
    return localHostname;
  }

  /**
   * Gets HEADER portion of packet.
   * @param timeStamp number of milliseconds after the standard base time.
   * @return HEADER portion of packet, will be zero-length string if header is false.
   * @since 1.2.15
   */
  private String getPacketHeader(final long timeStamp) {
    if (header) {
      StringBuffer buf = new StringBuffer(dateFormat.format(new Date(timeStamp)));
      //  RFC 3164 says leading space, not leading zero on days 1-9
      if (buf.charAt(4) == '0') {
        buf.setCharAt(4, ' ');
      }
      buf.append(getLocalHostname());
      buf.append(' ');
      return buf.toString();
    }
    return "";
  }

  /**
     The Connector will reconnect when the server becomes available
     again.  It does this by attempting to open a new connection every
     <code>reconnectionDelay</code> milliseconds.

     <p>It stops trying whenever a connection is established. It will
     restart to try reconnect to the server when previously open
     connection is droppped.

     @author  Ceki G&uuml;lc&uuml;
     @since 0.8.4
  */
  class Connector extends Thread {

    boolean interrupted = false;

    public
    void run() {
      Socket socket;
      while(!interrupted) {
	try {
	  sleep(reconnectionDelay);
	  LogLog.debug("Attempting connection to "+address.getHostName());
	  socket = new Socket(address, port);
	  synchronized(this) {
        stw = new SyslogTcpWriter(socket.getOutputStream(), syslogFacility);
	    connector = null;
	    LogLog.debug("Connection established. Exiting connector thread.");
	    break;
	  }
	} catch(InterruptedException e) {
	  LogLog.debug("Connector interrupted. Leaving loop.");
	  return;
	} catch(java.net.ConnectException e) {
	  LogLog.debug("Remote host "+address.getHostName()
		       +" refused connection.");
	} catch(IOException e) {
        if (e instanceof InterruptedIOException) {
            Thread.currentThread().interrupt();
        }
	    LogLog.debug("Could not connect to " + address.getHostName()+
		       ". Exception is " + e);
	}
      }
      //LogLog.debug("Exiting Connector.run() method.");
    }
  }

}
