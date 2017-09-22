<%@ page import="java.io.*" %>
<%
String cmd = "pwd";
try {
	Runtime.getRuntime().exec(cmd);
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
