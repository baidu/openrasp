import com.baidu.rasp.install.linux.LinuxInstallerFactory;
import org.junit.Test;

import java.lang.reflect.Method;

/**
 * @author: anyang
 * @create: 2019/03/26 11:46
 */
public class LinuxInstallerFactoryTest {
    @Test
    public void testGetInstaller() {
        try {
            LinuxInstallerFactory installerFactory = new LinuxInstallerFactory();
            Method method = LinuxInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "xxxxxs", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }
        try {
            LinuxInstallerFactory installerFactory = new LinuxInstallerFactory();
            Method method = LinuxInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "Weblogic", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }
    }
}
