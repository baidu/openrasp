<%@ page import="java.io.*" %>
<%@ page import="java.util.*" %>
<%
	request.getParameter("test");
	String dirname = "/etc";

	File folder = new File(dirname);
	if (folder.isDirectory()) {
		File[] listOfFiles = folder.listFiles();
		for (File file : listOfFiles) {
		    out.println(file.getName());
		}
	}
%>
