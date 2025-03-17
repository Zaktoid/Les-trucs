interface Time
{
void beat();
}
class Clock implements Time
{
private String t = "tick";
public void beat()
{
System.out.println(t);
}
}
class NewClock extends Clock
{
private String t = "tock";
public void beat()
{
System.out.println(t);
}
}
public class Test
{
public static void main(String[] args)
{
Time c = new NewClock();
c.beat();
}
}
