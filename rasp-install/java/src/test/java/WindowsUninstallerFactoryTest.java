import com.baidu.rasp.uninstall.windows.WindowsUninstallerFactory;
import org.junit.Test;

/**
 * @author: anyang
 * @create: 2019/03/25 21:28
 */
public class WindowsUninstallerFactoryTest {
    @Test
    public void testGetUninstaller() {
        try {

            Utils.invokeMethod(new WindowsUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }
        try {

            Utils.invokeMethod(new WindowsUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "JBoss 4-6", "/Users/anyang/Desktop/jacoco/sum/jboss-5.0.1.GA/");
        } catch (Exception e) {
            //
        }
        try {

            Utils.invokeMethod(new WindowsUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "Resin", "/Users/anyang/Desktop/jacoco/sum/resin-4.0.56/");
        } catch (Exception e) {
            //
        }
        try {

            Utils.invokeMethod(new WindowsUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "Weblogic", "/home/weblogic/wls12213/user_projects/domains/base_domain");
        } catch (Exception e) {
            //
        }

        try {
            Utils.invokeMethod(new WindowsUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "tomcat", "/home/weblogic/wls12213/user_projects/domains/base_domain");
        } catch (Exception e) {
            //
        }
    }
}
