<%@ page import="java.io.*" %>
<%@ page import="ognl.Ognl" %>
<%
try {
    Object value = Ognl.parseExpression("java.lang.Runtime");
    out.println(value);
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
