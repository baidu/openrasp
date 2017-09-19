<%@ page contentType="text/html; charset=UTF-8" %>
<%@ page import="org.w3c.dom.*, javax.xml.parsers.*" %>
<%@ page import="org.xml.sax.InputSource" %>
<%@ page import="java.io.StringReader" %>
<%
String data = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n" +
    " <!DOCTYPE foo [  \n" +
    "   <!ELEMENT foo ANY >\n" +
    "   <!ENTITY xxe SYSTEM \"file:///etc/passwd\" >]><foo>&xxe;</foo>";
try {
    DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
    DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
    Document doc = docBuilder.parse(new InputSource(new StringReader(data)));

    NodeList RegistrationNo = doc.getElementsByTagName("foo");
    out.println(RegistrationNo.item(0).getFirstChild().getNodeValue());
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
