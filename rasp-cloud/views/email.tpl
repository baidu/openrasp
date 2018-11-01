<table border="1" cellspacing="0" cellpadding="5">
    <thead>
        <tr>
            <th>id</th>
            <th>attack time</th>
            <th>source</th>
            <th>target</th>
            <th>attack type</th>
            <th>status</th>
        </tr>
    </thead>
    <tbody>
        {{range .Alarms}}
            <tr>
                <td>{{.id}}</td>
                <td>{{.event_time}}</td>
                <td>{{.attack_source}}</td>
                <td>{{.target}}</td>
                <td>{{.attack_type}}</td>
                <td>{{.intercept_state}}</td>
            </tr>
        {{end}}
    </tbody>
</table>
<br>
There are another {{.Total}} alarms form app:{{.AppName}}, for details  <a href="{{.DetailedLink}}">{{.DetailedLink}}</a>

