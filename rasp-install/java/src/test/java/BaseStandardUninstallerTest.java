import com.baidu.rasp.uninstall.BaseStandardUninstaller;
import com.baidu.rasp.uninstall.linux.TomcatUninstaller;
import org.junit.Test;

import java.lang.reflect.Method;

/**
 * @author: anyang
 * @create: 2019/03/26 10:15
 */
public class BaseStandardUninstallerTest {
    @Test
    public void testCheckTomcatVersion() {
        try {
            BaseStandardUninstaller uninstaller = new TomcatUninstaller("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            Method method = BaseStandardUninstaller.class.getDeclaredMethod("checkTomcatVersion");
            method.setAccessible(true);
            method.invoke(uninstaller);
        } catch (Exception e) {
            //
        }
    }

    @Test
    public void testDelRaspFolder() {
        try {
            BaseStandardUninstaller uninstaller = new TomcatUninstaller("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            uninstaller.delRaspFolder("/ddseff");
        } catch (Exception e) {
            //
        }
    }

    @Test
    public void testUninstall() {
        try {
            BaseStandardUninstaller uninstaller = new TomcatUninstaller("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            uninstaller.uninstall();
        } catch (Exception e) {
            //
        }
    }
}

