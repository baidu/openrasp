<%@ page import="java.io.*" %>
<%@ page contentType="text/html; charset=UTF-8" %>
<%
    String filename = "/tmp/writeFileTest";

    FileOutputStream fileOut = new FileOutputStream(filename);
    fileOut.write("write file".getBytes());
    fileOut.flush();
    fileOut.close();
%>
