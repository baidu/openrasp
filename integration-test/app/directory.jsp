<%@ page import="java.io.*" %>
<%@ page import="java.util.*" %>
<%
String dirname = "/etc";
try {
	File folder = new File(dirname);
	if (folder.isDirectory()) {
		File[] listOfFiles = folder.listFiles();
		for (File file : listOfFiles) {
		    out.println(file.getName());
		}
	}
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
