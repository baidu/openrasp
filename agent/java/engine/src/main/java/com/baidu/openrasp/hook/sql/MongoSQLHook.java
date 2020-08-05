/*
 * Copyright 2017-2018 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.baidu.openrasp.hook.sql;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.google.gson.Gson;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.lang.reflect.Method;
import java.util.HashMap;

/**
 * @description: MongoDB的增删改查hook点
 * @author: anyang
 * @create: 2019/05/13 11:45
 */
@HookAnnotation
public class MongoSQLHook extends AbstractClassHook {
    private static final String SQL_TYPE_MONGODB = "mongodb";
    private String className;
    private String type;

    @Override
    public boolean isClassMatched(String className) {
        if ("com/mongodb/MongoCollectionImpl".equals(className)) {
            this.type = SQL_TYPE_MONGODB;
            this.className = className;
            return true;
        }
        return false;
    }

    @Override
    public String getType() {
        return "mongodb";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String findMethodDesc = "(Lorg/bson/conversions/Bson;Ljava/lang/Class;)Lcom/mongodb/client/FindIterable;";
        String updateMethodDesc = "(Lorg/bson/conversions/Bson;Lorg/bson/conversions/Bson;Lcom/mongodb/client/model/UpdateOptions;Z)Lcom/mongodb/client/result/UpdateResult;";
        String deleteMethodDesc = "(Lorg/bson/conversions/Bson;Z)Lcom/mongodb/client/result/DeleteResult;";
        String insertOneMethodDesc = "(Ljava/lang/Object;Lcom/mongodb/client/model/InsertOneOptions;)V";
        String insertManyMethodDesc = "(Ljava/lang/Object;Lcom/mongodb/client/model/InsertManyOptions;)V";
        String replaceMethodDesc = "(Lorg/bson/conversions/Bson;Ljava/lang/Object;)Lcom/mongodb/client/model/UpdateOptions;";
        String findOneAndDeleteMethodDesc = "(Lorg/bson/conversions/Bson;Lcom/mongodb/client/model/FindOneAndDeleteOptions;)Ljava/lang/Object;";
        String findOneAndReplaceMethodDesc = "(Lorg/bson/conversions/Bson;Ljava/lang/Object;Lcom/mongodb/client/model/FindOneAndReplaceOptions;)Ljava/lang/Object;";
        String findOneAndUpdateMethodDesc = "(Lorg/bson/conversions/Bson;Lorg/bson/conversions/Bson;Lcom/mongodb/client/model/FindOneAndUpdateOptions;)Ljava/lang/Object;";
        String aggregateMethodDesc = "(Ljava/util/List;Ljava/lang/Class;)Lcom/mongodb/client/AggregateIterable;";
        String countMethodDesc = "(Lorg/bson/conversions/Bson;Lcom/mongodb/client/model/CountOptions;)J";

        String findSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"find\",$1", String.class, String.class, String.class, Object.class);
        String updateSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkUpdate",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"update\",$0,$1", String.class, String.class, String.class, Object.class, Object.class);
        String deleteSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"delete\",$1", String.class, String.class, String.class, Object.class);
        String insertOneSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"insertOne\",$1", String.class, String.class, String.class, Object.class);
        String insertManySrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"insertMany\",$1", String.class, String.class, String.class, Object.class);
        String replaceSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"replace\",$1", String.class, String.class, String.class, Object.class);
        String findOneAndDeleteSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"findOneAndDelete\",$1", String.class, String.class, String.class, Object.class);
        String findOneAndReplaceSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"findOneAndReplace\",$1", String.class, String.class, String.class, Object.class);
        String findOneAndUpdateSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"findOneAndUpdate\",$1", String.class, String.class, String.class, Object.class);
        String aggregateSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkAggregate",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"aggregate\",$1", String.class, String.class, String.class, Object.class);
        String countSrc = getInvokeStaticSrc(MongoSQLHook.class, "checkOperation",
                "\"" + type + "\"" + ",\"" + className + "\"" + ",\"count\",$1", String.class, String.class, String.class, Object.class);

        insertBefore(ctClass, "find", findMethodDesc, findSrc);
        insertBefore(ctClass, "insertOne", insertOneMethodDesc, insertOneSrc);
        insertBefore(ctClass, "insertMany", insertManyMethodDesc, insertManySrc);
        insertBefore(ctClass, "delete", deleteMethodDesc, deleteSrc);
        insertBefore(ctClass, "replaceOne", replaceMethodDesc, replaceSrc);
        insertBefore(ctClass, "update", updateMethodDesc, updateSrc);
        insertBefore(ctClass, "findOneAndDelete", findOneAndDeleteMethodDesc, findOneAndDeleteSrc);
        insertBefore(ctClass, "findOneAndReplace", findOneAndReplaceMethodDesc, findOneAndReplaceSrc);
        insertBefore(ctClass, "findOneAndUpdate", findOneAndUpdateMethodDesc, findOneAndUpdateSrc);
        insertBefore(ctClass, "aggregate", aggregateMethodDesc, aggregateSrc);
        insertBefore(ctClass, "count", countMethodDesc, countSrc);
    }

    public static void checkOperation(String server, String className, String methodName, Object operation) {
        String json = Reflection.invokeStringMethod(operation, "toJson", new Class[]{});
        if (json != null) {
            checkSQL(server, className, methodName, json);
        }
    }

    public static void checkUpdate(String server, String className, String methodName, Object object, Object operation) {
        try {
            Class clazz = Thread.currentThread().getContextClassLoader().loadClass("org.bson.conversions.Bson");
            Method method = object.getClass().getDeclaredMethod("toBsonDocument", clazz);
            method.setAccessible(true);
            Object document = method.invoke(object, operation);
            String json = Reflection.invokeStringMethod(document, "toJson", new Class[]{});
            if (json != null) {
                checkSQL(server, className, methodName, json);
            }
        } catch (Exception e) {
            LogTool.traceHookWarn("get mongo update query failed: " + e.getMessage(), e);
        }
    }

    public static void checkAggregate(String server, String className, String methodName, Object operation) {
        String json = new Gson().toJson(operation);
        if (json != null) {
            checkSQL(server, className, methodName, json);
        }
    }

    public static void checkSQL(String server, String className, String methodName, String query) {
        HashMap<String, Object> params = new HashMap<String, Object>();
        params.put("server", server);
        params.put("query", query);
        params.put("class", className);
        params.put("method", methodName);
        HookHandler.doCheck(CheckParameter.Type.MONGO, params);
    }


}
