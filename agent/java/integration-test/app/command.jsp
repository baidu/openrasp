<%@ page import="java.io.*" %>
<%
    out.println(request.getParameter("test"));
    String cmd = "pwd";
	Runtime.getRuntime().exec(cmd);
%>
