import com.baidu.rasp.uninstall.linux.LinuxUninstallerFactory;
import org.junit.Test;

/**
 * @author: anyang
 * @create: 2019/03/25 21:13
 */
public class LinuxUninstallerFactoryTest {
    @Test
    public void testGetUninstaller() {
        try {
            Utils.invokeMethod(new LinuxUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "Weblogic", "/home/weblogic/wls12213/user_projects/domains/base_domain");
            Utils.invokeMethod(new LinuxUninstallerFactory(), "getUninstaller", new Class[]{String.class, String.class},
                    "tomcat", "/home/weblogic/wls12213/user_projects/domains/base_domain");
        } catch (Exception e) {
            //
        }
    }
}
