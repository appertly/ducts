<?hh
namespace Ducts;

use HackPack\HackUnit\Contract\Assert;
use Psr\Http\Message\ServerRequestInterface as Request;
use Psr\Http\Message\ResponseInterface as Response;

function testSomething(Request $request, Response $response, (function (Request,Response): Response) $next): Response
{
    return $response->withHeader('X-Unit-Test', 'something');
}

class RunTest
{
    <<Test>>
    public async function testRun1(Assert $assert): Awaitable<void>
    {
        $queue = ImmVector{
            (Request $a, Response $b, (function (Request,Response): Response) $c) ==> $b->withHeader("X-Unit-Test", "Bar")
        };
        $run = new Run($queue);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $run->__invoke($request, $response);
        $assert->string($out->getHeaderLine('X-Unit-Test'))->is("Bar");
    }

    <<Test>>
    public async function testRun2(Assert $assert): Awaitable<void>
    {
        $queue = ImmVector{
            inst_meth($this, 'testFoobar')
        };
        $run = new Run($queue);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $run->__invoke($request, $response);
        $assert->string($out->getHeaderLine('X-Unit-Test'))->is("foobar");
    }

    <<Test>>
    public async function testRun3(Assert $assert): Awaitable<void>
    {
        $queue = ImmVector{
            class_meth(__CLASS__, 'testBarfoo')
        };
        $run = new Run($queue);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $run->__invoke($request, $response);
        $assert->string($out->getHeaderLine('X-Unit-Test'))->is("barfoo");
    }

    <<Test>>
    public async function testRun4(Assert $assert): Awaitable<void>
    {
        $queue = ImmVector{
            fun('Ducts\testSomething')
        };
        $run = new Run($queue);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $run->__invoke($request, $response);
        $assert->string($out->getHeaderLine('X-Unit-Test'))->is("something");
    }

    <<Test>>
    public async function testRun5(Assert $assert): Awaitable<void>
    {
        $queue = Vector{
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                $a = $a->withAttribute('foo', 'bar');
                $r = $c($a, $b);
                $assert->bool($r->hasHeader('X-Out-Second'))->is(true);
                return $r->withHeader('X-Out-Third', '1');
            },
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                $assert->mixed($a->getAttribute('foo'))->identicalTo('bar');
                $a = $a->withAttribute('bar', 'baz');
                $r = $c($a, $b);
                $assert->bool($r->hasHeader('X-Out-First'))->is(true);
                return $r->withHeader('X-Out-Second', '1');
            },
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                $assert->mixed($a->getAttribute('bar'))->identicalTo('baz');
                $a = $a->withAttribute('baz', 'biz');
                $r = $c($a, $b);
                return $r->withHeader('X-Out-First', '1');
            }
        };
        $queue->reverse();
        $run = new Run($queue->immutable());
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $run->__invoke($request, $response);
        $assert->string($out->getHeaderLine('X-Out-Third'))->is("1");
    }

    <<Test>>
    public async function testRun6(Assert $assert): Awaitable<void>
    {
        $queue = Vector{
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                try {
                    $r = $c($a, $b);
                } catch (\Exception $e) {
                    // ok
                }
                return $b->withHeader('X-Out', '1');
            },
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                throw new \Exception("o_O");
            }
        };
        $queue->reverse();
        $run = new Run($queue->immutable());
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $run->__invoke($request, $response);
        $assert->string($out->getHeaderLine('X-Out'))->is("1");
    }

    public function testFoobar(Request $request, Response $response, (function (Request,Response): Response) $next): Response
    {
        return $response->withHeader('X-Unit-Test', 'foobar');
    }

    public static function testBarfoo(Request $request, Response $response, (function (Request,Response): Response) $next): Response
    {
        return $response->withHeader('X-Unit-Test', 'barfoo');
    }
}
