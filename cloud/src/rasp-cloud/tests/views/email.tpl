<table border="1" cellspacing="0" cellpadding="5">
    <thead>
        <tr>
            <th>报警时间</th>
            <th>报警类型</th>
            <th>攻击来源</th>
            <th>攻击目标</th>
            <th>拦截状态</th>
            <th>报警详情</th>
        </tr>
    </thead>
    <tbody>
        {{range .Alarms}}
            <tr>
                <td>{{.event_time}}</td>
                <td>{{.attack_type}}</td>
                <td>{{.attack_source}}</td>
                <td>{{.target}}</td>
                <td>{{.intercept_state}}</td>
                <td><a href="{{$.DetailedLink}}/{{.id}}">detail</a></td>
            </tr>
        {{end}}
    </tbody>
</table>
<br>

若要查看更多 "<b>{{.AppName}}</b>" 的报警，请点击这里
{{if .DetailedLink}}
    <a href="{{.DetailedLink}}">{{.DetailedLink}}</a>
{{else}}
    http://127.0.0.1:{{.HttpPort}}
{{end}}

