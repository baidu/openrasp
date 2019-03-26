import com.baidu.rasp.install.InstallerFactory;
import com.baidu.rasp.install.linux.LinuxInstallerFactory;
import org.junit.Test;

import java.io.File;

/**
 * @author: anyang
 * @create: 2019/03/25 20:42
 */
public class InstallerFactoryTest {
    @Test
    public void testGetInstaller() {
        try {
            InstallerFactory factory = new LinuxInstallerFactory();
            factory.getInstaller(new File("/tmp"));
        } catch (Exception e) {
            //
        }
        try {
            InstallerFactory factory = new LinuxInstallerFactory();
            factory.getInstaller(new File("/xxxys"));
        } catch (Exception e) {
            //
        }
    }
}
