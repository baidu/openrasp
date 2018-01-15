<%@ page import="org.apache.commons.httpclient.HttpClient" %>
<%@ page import="org.apache.commons.httpclient.methods.GetMethod" %>

<%@ page contentType="text/html; charset=UTF-8" %>
<%
	try {
	    HttpClient httpClient = new HttpClient();
	    GetMethod getMethod = new GetMethod("http://0x7f.0x0.0x0.0x1:8080/app");
	    httpClient.executeMethod(getMethod);

	    String charSet = getMethod.getResponseCharSet();
	    byte[] responseBody = getMethod.getResponseBody();
	    out.println("response:\r\n" + new String(responseBody, charSet));
	} catch (Exception e) {
	    out.println(e.getMessage());
	}
%>
