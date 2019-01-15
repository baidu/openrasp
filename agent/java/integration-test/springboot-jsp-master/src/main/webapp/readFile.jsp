<%@ page import="java.io.*" %>
<%@ page contentType="text/html; charset=UTF-8" %>
<%
    String filename = "/etc/passwd";

    FileInputStream fileIn = new FileInputStream(filename);
    BufferedReader reader = new BufferedReader(new FileReader(filename));
    String line = reader.readLine();
    while (line != null) {
        out.println(line);
        line = reader.readLine();
    }
    fileIn.close();
%>
