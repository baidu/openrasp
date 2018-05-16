<%@ page contentType="text/html; charset=UTF-8" %>
<%@ page import="java.io.*,java.util.*,java.sql.*" %>
<%@ page import="java.sql.*" %>
<%
    Class.forName("org.sqlite.JDBC");
    Connection conn = DriverManager.getConnection("jdbc:sqlite::memory");
    try {
        String createSql = "CREATE TABLE user(NAME TEXT NOT NULL)";
        Statement statement = conn.createStatement();
        statement.execute(createSql);
    } catch (Exception e) {
    }
    String sql = "SELECT * FROM user";
    PreparedStatement stmt = conn.prepareStatement(sql);
    ResultSet rs = stmt.executeQuery();
    while (rs.next()) {
        out.println(rs.getString("user"));
    }
%>
