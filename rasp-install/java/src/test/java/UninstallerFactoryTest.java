import com.baidu.rasp.uninstall.UninstallerFactory;
import com.baidu.rasp.uninstall.linux.LinuxUninstallerFactory;
import org.junit.Test;

import java.io.File;

/**
 * @author: anyang
 * @create: 2019/03/25 21:35
 */
public class UninstallerFactoryTest {
    @Test
    public void testGetInstaller() {
        try {
            UninstallerFactory factory = new LinuxUninstallerFactory();
            factory.getUninstaller(new File("/tmp"));
        } catch (Exception e) {
            //
        }

        try {
            UninstallerFactory factory = new LinuxUninstallerFactory();
            factory.getUninstaller(new File("/xxxys"));
        } catch (Exception e) {
            //
        }
    }
}
