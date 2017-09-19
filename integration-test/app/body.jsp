<%@ page import="java.io.*" %>
<%
try {
    BufferedReader reader = request.getReader();
    String input = null;
    while((input = reader.readLine()) != null){
        out.println(input);
    }
    // Runtime.getRuntime().exec("ls");
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
