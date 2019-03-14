<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <title>[OpenRASP] 邮件报警</title>
        <meta name="viewport" content="width=device-width" />
       <style type="text/css">
            @media only screen and (max-width: 550px), screen and (max-device-width: 550px) {
                body[yahoo] .buttonwrapper { background-color: transparent !important; }
                body[yahoo] .button { padding: 0 !important; }
                body[yahoo] .button a { background-color: #148e81; padding: 15px 25px !important; }
            }

            @media only screen and (min-device-width: 601px) {
                .content { width: 600px !important; }
                .col387 { width: 387px !important; }
            }
        </style>
    </head>
    <body bgcolor="#352738" style="margin: 0; padding: 0;" yahoo="fix">
        <!--[if (gte mso 9)|(IE)]>
        <table width="600" align="center" cellpadding="0" cellspacing="0" border="0">
          <tr>
            <td>
        <![endif]-->
        <table align="center" border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse; width: 100%; max-width: 600px;" class="content">
            <tr>
                <td align="center" bgcolor="#148e81" style="padding: 20px 20px 20px 20px; color: #ffffff; font-family: Arial, sans-serif; font-size: 36px; font-weight: bold;">
                    攻击事件
                </td>
            </tr>            
            {{range .Alarms}}
            <tr>
                <td bgcolor="#ffffff" style="padding: 20px 20px 10px 20px; color: #555555; font-family: Arial, sans-serif; font-size: 20px; line-height: 30px;">
                    <b style="word-break: break-all;"> {{.index}}. [{{.attack_type}}] {{.domain}}</b>
                </td>
            </tr>
            <tr>
                <td bgcolor="#ffffff" style="padding: 0 20px 20px 20px; color: #555555; font-family: Arial, sans-serif; font-size: 15px; line-height: 24px; border-bottom: 1px solid #f6f6f6;">
                    <dl>
                        <dt style="float: left"><b>攻击时间: </b></dt>
                        <dd style="margin-left: 70px;">{{.event_time}}</dd>

                        <dt style="float: left"><b>攻击类型: </b></dt>
                        <dd style="margin-left: 70px;">{{.attack_type}}</dd>

                        <dt style="float: left"><b>拦截状态: </b></dt>
                        <dd style="margin-left: 70px;">{{.intercept_state}}</dd>

                        <dt style="float: left"><b>攻击来源: </b></dt>
                        <dd style="margin-left: 70px;">{{.attack_source}}</dd>

                        <dt style="float: left"><b>攻击目标: </b></dt>
                        <dd style="margin-left: 70px;">{{.url}}</dd>
                    </dl>
                </td>
            </tr>
            {{end}}

            <tr>
                <td align="center" bgcolor="#f9f9f9" style="padding: 30px 20px 30px 20px; font-family: Arial, sans-serif;">
                    <table bgcolor="#148e81" border="0" cellspacing="0" cellpadding="0" class="buttonwrapper">
                        <tr>
                            <td align="center" height="50" style=" padding: 0 25px 0 25px; font-family: Arial, sans-serif; font-size: 16px; font-weight: bold;" class="button">
                                {{if .DetailedLink}}
                                    <a href="{{.DetailedLink}}" style="color: #ffffff; text-align: center; text-decoration: none;">查看 {{.AppName}} 所有报警</a>
                                {{else}}
                                    http://127.0.0.1:{{.HttpPort}}
                                {{end}}
                            </td>
                        </tr>
                    </table>
                </td>
            </tr>
            <tr>
                <td align="center" bgcolor="#dddddd" style="padding: 15px 10px 15px 10px; color: #555555; font-family: Arial, sans-serif; font-size: 12px; line-height: 18px;">
                    Powered by <a href="https://rasp.baidu.com">OpenRASP</a>
                </td>
            </tr>
        </table>
        <!--[if (gte mso 9)|(IE)]>
                </td>
            </tr>
        </table>
        <![endif]-->
    </body>
</html>