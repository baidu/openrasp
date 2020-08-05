import com.baidu.rasp.install.windows.TomcatInstaller;
import org.apache.commons.io.IOUtils;
import org.junit.Test;

import static org.junit.Assert.*;

import java.io.File;
import java.io.FileReader;
import java.io.InputStream;
import java.lang.reflect.Constructor;

/**
 * @author: anyang
 * @create: 2019/03/25 15:57
 */
public class WindowsTomcatInstallerTest {
    @Test
    public void testGetInstallPath() {
        try {
            Class clazz = TomcatInstaller.class;
            Constructor constructor = clazz.getConstructor(String.class, String.class);
            Object installer = constructor.newInstance("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            String path = Utils.invokeStringMethod(installer, "getInstallPath", new Class[]{String.class},
                    "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30");
            assertEquals(path, "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30\\rasp");
        } catch (Exception e) {
            //
        }
    }

    @Test
    public void testGetScript() {
        try {
            Class clazz = TomcatInstaller.class;
            Constructor constructor = clazz.getConstructor(String.class, String.class);
            Object installer = constructor.newInstance("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            String path = Utils.invokeStringMethod(installer, "getScript", new Class[]{String.class},
                    "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30\\rasp");
            assertEquals(path, "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30\\rasp\\..\\bin\\catalina.bat");
        } catch (Exception e) {
            //
        }
    }

    @Test
    public void testModifyStartScript() {
        try {
            String content = IOUtils.toString(new FileReader(new File("/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/bin/catalina.bat")));
            Class clazz = TomcatInstaller.class;
            Constructor constructor = clazz.getConstructor(String.class, String.class);
            Object installer = constructor.newInstance("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            Utils.invokeStringMethod(installer, "modifyStartScript", new Class[]{String.class}, content);
        } catch (Exception e) {
            //
        }

        try {
            InputStream is = this.getClass().getResourceAsStream("/startWebLogic.sh");
            String content = IOUtils.toString(is, "UTF-8");
            Class clazz = TomcatInstaller.class;
            Constructor constructor = clazz.getConstructor(String.class, String.class);
            Object installer = constructor.newInstance("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            Utils.invokeStringMethod(installer, "modifyStartScript", new Class[]{String.class}, content);
        } catch (Exception e) {
            //
        }
    }
}
