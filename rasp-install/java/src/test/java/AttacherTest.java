import com.baidu.rasp.Attacher;
import org.junit.Test;

/**
 * @author: anyang
 * @create: 2019/03/26 10:22
 */
public class AttacherTest {
    @Test
    public void testDoAttach() {
        try {
            Attacher attacher = new Attacher("4567", "/Users/anyang/Desktop/jacoco/sum/jboss-5.0.1.GA/");
            attacher.doAttach("install");
        } catch (Throwable e) {
            //
        }

        try {
            Attacher attacher = new Attacher("24682", "/Users/anyang/Desktop/jacoco/sum/jboss-5.0.1.GA/");
            attacher.doAttach("uninstall");
        } catch (Throwable e) {
            //
        }
    }
}
