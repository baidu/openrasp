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
        }
        catch (Exception e){
            e.printStackTrace();
        }
    }

    public static void test() throws Exception{
        JsonObject config = getConfig();
        JsonObject requestInfo;
        JsonObject params;
        String input = readJsonFile(UnitTest.class.getResource("/pluginUnitTest/unitCases.json").getPath());
        Gson gson = new Gson();
        Iterator testCases = gson.fromJson(input, JsonElement.class).getAsJsonArray().iterator();
        while(testCases.hasNext()) {
            requestInfo = (JsonObject)testCases.next();
            params = (JsonObject)requestInfo.get("params");

            Map<String, Object> paramMap = new HashMap<String, Object>();
            for (Map.Entry<String, JsonElement> entrySet : params.entrySet()) {
                if(entrySet.getValue().isJsonArray()){
                    Iterator<JsonElement> stringArray = entrySet.getValue().getAsJsonArray().iterator();
                    LinkedList<String> stringArrayList = new LinkedList<String>();
                    while(stringArray.hasNext()){
                        stringArrayList.add(stringArray.next().getAsString());
                    }
                    paramMap.put(entrySet.getKey(), stringArrayList);
                }
                else{
                    paramMap.put(entrySet.getKey(), entrySet.getValue().getAsString());
                }
            }
            UnitTestRequest testRequest = new UnitTestRequest(requestInfo);
            CheckParameter checkParameter = new CheckParameter(null, paramMap);
            String testUnitName = requestInfo.get("name").getAsString();
            List<EventInfo> result;
            if(testUnitName.equals("sql")){
                SqlStatementChecker sqlStatementChecker = new SqlStatementChecker();
                result = sqlStatementChecker.testCheckSql(checkParameter, testRequest.getParameterMap(), config);
            }
            else if(testUnitName.equals("ssrf")){
                SSRFChecker ssrfChecker = new SSRFChecker();
                result = ssrfChecker.testCheckSSRF(checkParameter, testRequest.getParameterMap(), config);
            }
            else{
                System.out.println("[IGNORED] Test id:" + requestInfo.get("id").getAsString());
                continue;
            }
            String action = requestInfo.get("action").getAsString();
            if(((action.equals("block") || action.equals("log")) && result.size() != 0)
                    || (action.equals("ignore")) && result.size() == 0){
                System.out.println("[PASS] Test id:" + requestInfo.get("id").getAsString() );
            }
            else {
                System.out.println("[FAILED] Test id:" + requestInfo.get("id").getAsString());
            }
        }
    }

    private static String readJsonFile(String path) throws IOException{
        File file = new File(path);
        FileReader reader = new FileReader(file);
        int fileLen = (int)file.length();
        char[] chars = new char[fileLen];
        reader.read(chars);
        return String.valueOf(chars);
    }

    public static JsonObject getConfig() throws Exception{
        String configJson = readJsonFile(UnitTest.class.getResource("/pluginUnitTest/unitConfig.json").getPath());
        Gson gson = new Gson();
        return gson.fromJson(configJson, JsonElement.class).getAsJsonObject();
    }


}
