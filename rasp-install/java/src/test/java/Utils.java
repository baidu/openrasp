import java.lang.reflect.Method;

/**
 * @author: anyang
 * @create: 2019/03/25 16:20
 */
public class Utils {

    public static Object invokeStaticMethod(String className, String methodName, Class[] paramTypes, Object... parameters) {
        try {
            Class clazz = Class.forName(className);
            return invokeMethod(null, clazz, methodName, paramTypes, parameters);
        } catch (Exception e) {
            return null;
        }
    }

    public static String invokeStringMethod(Object object, String methodName, Class[] paramTypes, Object... parameters) {
        Object ret = invokeMethod(object, methodName, paramTypes, parameters);
        return ret != null ? (String) ret : null;
    }

    public static Object invokeMethod(Object object, String methodName, Class[] paramTypes, Object... parameters) {
        if (object == null) {
            return null;
        }
        return invokeMethod(object, object.getClass(), methodName, paramTypes, parameters);
    }

    public static Object invokeMethod(Object object, Class clazz, String methodName, Class[] paramTypes, Object... parameters) {
        try {
            Method method = clazz.getDeclaredMethod(methodName, paramTypes);
            if (!method.isAccessible()) {
                method.setAccessible(true);
            }
            return method.invoke(object, parameters);
        } catch (Exception e) {
            return null;
        }
    }


}
