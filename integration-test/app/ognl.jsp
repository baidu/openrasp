<%@ page import="java.io.*" %>
<%@ page import="ognl.Ognl" %>
<%
    Object value = Ognl.parseExpression("java.lang.Runtime");
    out.println(value);
%>
