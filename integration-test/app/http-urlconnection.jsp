<%@ page import="java.io.BufferedReader" %>
<%@ page import="java.net.URL" %>
<%@ page import="java.net.URLConnection" %>
<%@ page import="java.io.InputStream" %>
<%@ page import="java.io.InputStreamReader" %>

<%@ page contentType="text/html; charset=UTF-8" %>
<%
        try {
            URL url = new URL("http://0x7f.0x0.0x0.0x1:8080/app");
            URLConnection urlConnection = url.openConnection();
            urlConnection.connect();
            InputStream inputStream = urlConnection.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
            String line = reader.readLine();
            String content = "";
            while (line != null) {
                content += (line + "\n");
                line = reader.readLine();
            }
            out.println(content);
        } catch (Exception e) {
            out.println(e.getMessage());
        }
%>
