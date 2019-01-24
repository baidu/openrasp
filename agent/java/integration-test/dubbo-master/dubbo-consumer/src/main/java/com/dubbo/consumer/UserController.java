package com.dubbo.consumer;

import com.dubbo.demo_interface.api.IUserService;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;

/**
 * Created by anyang on 2018/6/25.
 */
@Controller
public class UserController {

    @Resource
    private IUserService userService;

    @RequestMapping("/mysql")
    public String find(HttpServletRequest request) throws Exception {
        String result = userService.select();
        request.setAttribute("result", result);
        return "main";
    }

}
