<%@ page import="org.apache.http.HttpResponse" %>
<%@ page import="org.apache.http.client.HttpClient" %>
<%@ page import="org.apache.http.client.methods.HttpGet" %>
<%@ page import="org.apache.http.impl.client.HttpClients" %>
<%@ page import="java.io.BufferedReader" %>
<%@ page import="java.io.InputStreamReader" %>
<%@ page import="java.util.Map" %>

<%@ page contentType="text/html; charset=UTF-8" %>
<%

        try {
            StringBuffer resultBuffer = null;
            HttpClient client = HttpClients.createDefault();
            BufferedReader br = null;
            HttpGet httpGet = new HttpGet("http://0x7f.0x0.0x0.0x1:8080/app");
            HttpResponse res = client.execute(httpGet);
            // 读取服务器响应数据
            br = new BufferedReader(new InputStreamReader(res.getEntity().getContent()));
            String temp;
            resultBuffer = new StringBuffer();
            while ((temp = br.readLine()) != null) {
                resultBuffer.append(temp);
            }
            out.println(resultBuffer);
        } catch (Exception e) {
            out.println(e.getMessage());
        }

%>
