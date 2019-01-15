<%@ page contentType="text/html; charset=UTF-8" %>
<%@ page import="java.io.*,java.util.*,java.sql.*" %>
<%@ page import="java.sql.*" %>
<%
    Class.forName("org.sqlite.JDBC");
    Connection conn = DriverManager.getConnection("jdbc:sqlite::memory");
    Statement stmt = conn.createStatement();
    String sql = "SELECT * FROM user";
    ResultSet rs = stmt.executeQuery(sql);
    while (rs.next()) {
        out.println(rs.getString("user"));
    }
%>
