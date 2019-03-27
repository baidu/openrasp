import com.baidu.rasp.install.BaseStandardInstaller;
import org.junit.Test;

import static org.junit.Assert.*;

/**
 * @author: anyang
 * @create: 2019/03/25 20:29
 */
public class BaseStandardInstallerTest {
    @Test
    public void testModifyFolerPermission() {
        try {
            String temp = System.getProperty("user.name");
            System.setProperty("user.name", "root");
            Utils.invokeStaticMethod("com.baidu.rasp.install.BaseStandardInstaller", "modifyFolerPermission", new Class[]{String.class}, "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            System.setProperty("user.name", temp);

            temp = System.getProperty("os.name");
            System.setProperty("os.name", "Windows");
            Utils.invokeStaticMethod("com.baidu.rasp.install.BaseStandardInstaller", "modifyFolerPermission", new Class[]{String.class}, "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            System.setProperty("os.name", temp);
        } catch (Exception e) {
            //
        }
    }

    @Test
    public void testRunCommand() {
        try {
            String res = BaseStandardInstaller.runCommand(new String[]{"pwd"});
            assertNotNull(res);
        } catch (Exception e) {
            //
        }
    }

    @Test
    public void testFindFile() {
        try {
            Object object = Utils.invokeStaticMethod("com.baidu.rasp.install.BaseStandardInstaller", "findFile", new Class[]{String.class, String.class}, "/sdfsf", "baidu");
            assertNull(object);
        } catch (Exception e) {
            //
        }
    }
}
