<%@ page import="java.io.*" %>
<%@ page contentType="text/html; charset=UTF-8" %>
<%
    out.println(request.getParameter("test"));
    String filename = "/tmp/writeFileTest";

    FileOutputStream fileOut = new FileOutputStream(filename);
    fileOut.write("write file".getBytes());
    fileOut.flush();
    fileOut.close();
%>
