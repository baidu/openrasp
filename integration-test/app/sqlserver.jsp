<%@ page contentType="text/html; charset=UTF-8" %>
<%@ page import="java.io.*,java.util.*,java.sql.*" %>
<%@ page import="java.sql.*" %>
<%
    Class.forName("com.microsoft.sqlserver.jdbc.SQLServerDriver");
    Connection conn = DriverManager.getConnection("jdbc:sqlserver://127.0.0.1;databaseName=master;user=sa;password=yourStrong(!)Password;loginTimeout=5");
    Statement stmt = conn.createStatement();
    String sql = "SELECT * FROM user";
    ResultSet rs = stmt.executeQuery(sql);
    while (rs.next()) {
        out.println(rs.getString("user"));
    }
%>
