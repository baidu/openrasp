import com.baidu.rasp.install.windows.ResinInstaller;
import org.apache.commons.io.IOUtils;
import org.junit.Test;

import java.io.InputStream;
import java.lang.reflect.Constructor;

/**
 * @author: anyang
 * @create: 2019/03/26 12:08
 */
public class WindowsResinInstallerTest {
    @Test
    public void testModifyStartScript() {
        try {
            InputStream is = this.getClass().getResourceAsStream("/startWebLogic.sh");
            String content = IOUtils.toString(is, "UTF-8");
            Class clazz = ResinInstaller.class;
            Constructor constructor = clazz.getConstructor(String.class, String.class);
            Object installer = constructor.newInstance("Tomcat", "/Users/anyang/Desktop/jacoco/sum/apache-tomcat-8.5.30/");
            Utils.invokeStringMethod(installer, "modifyStartScript", new Class[]{String.class}, content);
        } catch (Exception e) {
            //
        }
    }
}
