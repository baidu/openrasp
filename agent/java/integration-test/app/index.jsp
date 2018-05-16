<%@ page import="java.io.*" %>
<%@ page import="java.util.*" %>
<%@ page contentType="text/html; charset=UTF-8" %>
<html>
<head>
	<meta charset="UTF-8"/>
	<title>Index</title>
</head>
<body>
<%
	String curr = getServletContext().getRealPath("/");
	File[] files = new File(curr).listFiles();
	Arrays.sort(files);
	for (int i = 0; i < files.length; i++) {
		if (files[i].isFile()) {
		%>
		<a href=<%= files[i].getName() %>><%= files[i].getName()%></a><br>
		<%
		}
	}
%>
</body>
</html>
