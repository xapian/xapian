import java.io.*;

public class WriteJavaVersion
{
    public static void main(String[] args)
    {
        try {
            File f = new File("javaversion.h");
            BufferedWriter w = new BufferedWriter(new FileWriter(f));
            w.write("#define JAVA_VERSION \"" +
                System.getProperty("java.version") + "\"\n");
            w.close();
        }
        catch (IOException e)
        {
            System.out.println("ERROR " + e);
        }
    }
} 