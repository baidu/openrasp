<%@ page import="java.io.*" %>
<%@ page contentType="text/html; charset=UTF-8" %>
<%
String filename = "/tmp/writeFileTest";
try {
    FileOutputStream fileOut = new FileOutputStream(filename);
    fileOut.write("write file".getBytes());
    fileOut.flush();
    fileOut.close();
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
