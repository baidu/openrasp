<%@ page import="java.io.*" %>
<%
    out.println(request.getParameter("test"));
    BufferedReader reader = request.getReader();
    String input = null;
    while((input = reader.readLine()) != null){
        out.println(input);
    }
    // Runtime.getRuntime().exec("ls");
%>
