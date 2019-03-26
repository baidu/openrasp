import com.baidu.rasp.install.windows.WindowsInstallerFactory;
import org.junit.Test;

import java.lang.reflect.Method;

/**
 * @author: anyang
 * @create: 2019/03/26 11:03
 */
public class WindowsInstallerFactoryTest {
    @Test
    public void testGetInstaller() {
        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }

        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "JBoss 4-6", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }

        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "Resin", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }
        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "Weblogic", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }
        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "JbossEAP", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }

        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "Wildfly", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }

        try {
            WindowsInstallerFactory installerFactory = new WindowsInstallerFactory();
            Method method = WindowsInstallerFactory.class.getDeclaredMethod("getInstaller", String.class, String.class);
            method.setAccessible(true);
            method.invoke(installerFactory, "xxxxxs", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
        } catch (Exception e) {
            //
        }
    }
}
