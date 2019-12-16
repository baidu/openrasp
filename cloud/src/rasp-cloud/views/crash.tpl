<table border="1" cellspacing="5" cellpadding="5">
    <thead>
        <tr>
            {{if .Time}}
            <th>time</th>
            {{end}}
            {{if .AppName}}
            <th>app name</th>
            {{end}}
            <th>hostname</th>
            <th>language</th>
            {{if .Ip}}
            <th>ip</th>
            {{end}}
            {{if .Version}}
            <th>rasp version</th>
            {{end}}
        </tr>
    </thead>
    <tbody>
        <tr>
            {{if .Time}}
            <th>{{.Time}}</th>
            {{end}}
            {{if .AppName}}
            <th>{{.AppName}}</th>
            {{end}}
            <td>{{.Hostname}}</td>
            <td>{{.Language}}</td>
            {{if .Ip}}
            <td>{{.Ip}}</td>
            {{end}}
            {{if .Version}}
            <th>{{.Version}}</th>
            {{end}}
        </tr>
    </tbody>
</table>