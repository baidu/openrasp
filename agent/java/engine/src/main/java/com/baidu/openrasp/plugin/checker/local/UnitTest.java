package com.baidu.openrasp.plugin.checker.local;

import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.request.UnitTestRequest;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import java.io.*;
import java.util.*;

public class UnitTest {

    public static void main(String[] args) {
        try {
            test();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void test() throws Exception {
        JsonObject config = getConfig();
        JsonObject requestInfo;
        JsonObject params;
        int success = 0, fail = 0, ignore = 0;
        LinkedList<String> testCaseFiles = getJsonFiles(UnitTest.class.getResource("/pluginUnitTest/unitCases").getPath());
        for (String fileName : testCaseFiles) {

            String input = readJsonFile(fileName);
            Gson gson = new Gson();
            for (Object o : gson.fromJson(input, JsonElement.class).getAsJsonArray()) {
                requestInfo = (JsonObject) o;
                params = (JsonObject) requestInfo.get("params");
                //获取case中的param
                Map<String, Object> paramMap = new HashMap<String, Object>();
                for (Map.Entry<String, JsonElement> entrySet : params.entrySet()) {
                    if (entrySet.getValue().isJsonArray()) {
                        Iterator<JsonElement> stringArray = entrySet.getValue().getAsJsonArray().iterator();
                        LinkedList<String> stringArrayList = new LinkedList<String>();
                        while (stringArray.hasNext()) {
                            stringArrayList.add(stringArray.next().getAsString());
                        }
                        paramMap.put(entrySet.getKey(), stringArrayList);
                    } else {
                        paramMap.put(entrySet.getKey(), entrySet.getValue().getAsString());
                    }
                }
                //构造AbstractRequest
                UnitTestRequest testRequest = new UnitTestRequest(requestInfo);
                //构造CheckParameter
                CheckParameter checkParameter = new CheckParameter(null, paramMap);

                //根据测试类型，调用对应的检测方法
                String testUnitName = requestInfo.get("name").getAsString();
                List<EventInfo> result;
                if (testUnitName.equals("sql")) {
                    SqlStatementChecker sqlStatementChecker = new SqlStatementChecker();
                    result = sqlStatementChecker.testCheckSql(checkParameter, testRequest.getParameterMap(), config);
                } else if (testUnitName.equals("ssrf")) {
                    SSRFChecker ssrfChecker = new SSRFChecker();
                    result = ssrfChecker.testCheckSSRF(checkParameter, testRequest.getParameterMap(), config);
                } else {
                    //忽略没有对应方法的用例
                    //System.out.println("[IGNORED] Test id:" + requestInfo.get("id").getAsString());
                    ignore++;
                    continue;
                }
                String action = requestInfo.get("action").getAsString();

                if (((action.equals("block") || action.equals("log")) && result.size() != 0)
                        || (action.equals("ignore")) && result.size() == 0) {
                    success++;
                    //System.out.println("[PASS] Test id:" + requestInfo.get("id").getAsString());
                } else {
                    fail++;
                    System.out.println("[FAILED] Test id:" + requestInfo.get("id").getAsString());
                    System.out.println("Case description:" + requestInfo.get("description").getAsString());
                }
            }
        }
        String output = String.format("Test finished. %d cases succeed, %d cases failed, %d cases ignored.", success, fail, ignore);
        System.out.println(output);
    }

    private static String readJsonFile(String path) throws IOException {
        File file = new File(path);
        FileReader reader = new FileReader(file);
        int fileLen = (int) file.length();
        char[] chars = new char[fileLen];
        reader.read(chars);
        return String.valueOf(chars);
    }

    public static LinkedList<String> getJsonFiles(String pathName) throws IOException {
        File dirFile = new File(pathName);
        if (!dirFile.exists() || !dirFile.isDirectory()) {
            throw new IOException();
        }
        String[] fileList = dirFile.list();
        LinkedList<String> result = new LinkedList<String>();
        for (String string : fileList) {
            File file = new File(dirFile.getPath(), string);
            if (!file.isDirectory()) {
                result.push(dirFile.getPath() + File.separator + string);
            }
        }
        return result;
    }


    public static JsonObject getConfig() throws Exception {
        String configJson = readJsonFile(UnitTest.class.getResource("/pluginUnitTest/unitConfig.json").getPath());
        Gson gson = new Gson();
        return gson.fromJson(configJson, JsonElement.class).getAsJsonObject();
    }


}
