<%@ page contentType="text/html; charset=UTF-8" %>
<%@ page import="java.io.*,java.util.*,java.sql.*" %>
<%@ page import="java.sql.*" %>
<%
try {
    Class.forName("org.postgresql.Driver");
    Connection conn = DriverManager.getConnection("jdbc:postgresql://postgresql/postgres", "postgres", "mysecretpassword");
    Statement stmt = conn.createStatement();
    String sql = "SELECT * FROM user";
    ResultSet rs = stmt.executeQuery(sql);
    while (rs.next()) {
        out.println(rs.getString("user"));
    }
} catch (Exception e) {
    if (e.getClass().getName().equals("com.fuxi.javaagent.exception.SecurityException")) {
        response.sendError(403, e.getMessage());
    } else {
        throw e;
    }
}
%>
