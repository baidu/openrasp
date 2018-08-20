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
        String input = "";


        try {
            input = readJsonFile(UnitTest.class.getResource("/unit.json").getPath());
        }
        catch (IOException ioe) {
            ioe.printStackTrace();
        }
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
                result = new ArrayList<EventInfo>();
            }
            String action = requestInfo.get("action").getAsString();
            if(((action.equals("block") || action.equals("log")) && result.size() != 0)
                    || (action.equals("ignore")) && result.size() == 0){
                System.out.println("Test id:" + requestInfo.get("id").getAsString() + " passed!");
            }
            else {
                System.out.println("Test id:" + requestInfo.get("id").getAsString() + " failed!");
            }

            System.out.println(0);
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

    public static JsonObject getConfig() {
        String configJson = "{\"cache\":{\"sqli\":{\"capacity\":100}},\"sqli_userinput\":{\"action\":\"block\",\"min_length\":15},\"sqli_dbmanager\":{\"action\":\"ignore\"},\"sqli_policy\":{\"action\":\"block\",\"feature\":{\"stacked_query\":true,\"no_hex\":true,\"version_comment\":true,\"function_blacklist\":true,\"union_null\":true,\"constant_compare\":false,\"into_outfile\":true},\"function_blacklist\":{\"load_file\":true,\"benchmark\":true,\"sleep\":true,\"pg_sleep\":true,\"is_srvrolemember\":true,\"updatexml\":true,\"extractvalue\":true,\"hex\":true,\"char\":true,\"chr\":true,\"mid\":true,\"ord\":true,\"ascii\":true,\"bin\":true}},\"ssrf_userinput\":{\"action\":\"block\"},\"ssrf_aws\":{\"action\":\"block\"},\"ssrf_common\":{\"action\":\"block\",\"domains\":[\".ceye.io\",\".vcap.me\",\".xip.name\",\".xip.io\",\".nip.io\",\".burpcollaborator.net\",\".tu4.org\"]},\"ssrf_obfuscate\":{\"action\":\"block\"},\"ssrf_protocol\":{\"action\":\"block\",\"protocols\":[\"file\",\"gopher\",\"jar\",\"netdoc\",\"dict\",\"php\",\"phar\",\"compress.zlib\",\"compress.bzip2\"]},\"readFile_userinput\":{\"action\":\"block\"},\"readFile_userinput_http\":{\"action\":\"block\"},\"readFile_userinput_unwanted\":{\"action\":\"block\"},\"readFile_traversal\":{\"action\":\"block\"},\"readFile_unwanted\":{\"action\":\"block\"},\"writeFile_NTFS\":{\"action\":\"block\"},\"writeFile_PUT_script\":{\"action\":\"block\"},\"writeFile_script\":{\"action\":\"log\"},\"rename_webshell\":{\"action\":\"block\"},\"copy_webshell\":{\"action\":\"block\"},\"directory_reflect\":{\"action\":\"block\"},\"directory_unwanted\":{\"action\":\"block\"},\"directory_outsideWebroot\":{\"action\":\"block\"},\"include_protocol\":{\"action\":\"block\",\"protocols\":[\"file\",\"gopher\",\"jar\",\"netdoc\",\"http\",\"https\",\"dict\",\"php\",\"phar\",\"compress.zlib\",\"compress.bzip2\"]},\"include_dir\":{\"action\":\"block\"},\"include_unwanted\":{\"action\":\"block\"},\"include_outsideWebroot\":{\"action\":\"block\"},\"xxe_protocol\":{\"action\":\"block\",\"protocols\":[\"ftp\",\"dict\",\"gopher\",\"jar\",\"netdoc\"]},\"xxe_file\":{\"action\":\"log\"},\"fileUpload_webdav\":{\"action\":\"block\"},\"fileUpload_multipart\":{\"action\":\"block\"},\"ognl_exec\":{\"action\":\"block\"},\"command_reflect\":{\"action\":\"block\"},\"command_userinput\":{\"action\":\"block\"},\"command_other\":{\"action\":\"log\"},\"transformer_deser\":{\"action\":\"block\"}}";
        Gson gson = new Gson();
        return gson.fromJson(configJson, JsonElement.class).getAsJsonObject();
    }


}
