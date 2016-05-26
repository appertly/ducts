<?hh
namespace Ducts;

use HackPack\HackUnit\Contract\Assert;
use Psr\Http\Message\ServerRequestInterface as Request;
use Psr\Http\Message\ResponseInterface as Response;

class RunnerTest
{
    <<Test>>
    public async function testRun(Assert $assert): Awaitable<void>
    {
        $queue = [
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
        ];
        $runner = new Runner($queue);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $runner->run($request, $response);
        $assert->string($out->getHeaderLine('X-Out-Third'))->is("1");
        $out = $runner->run($request, $response);
        $assert->string($out->getHeaderLine('X-Out-Third'))->is("1");
    }

    <<Test>>
    public async function testLayer(Assert $assert): Awaitable<void>
    {
        $queue = [
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
            }
        ];
        $runner = new Runner($queue);
        $queue2 = [
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                $r = $c($a, $b);
                $assert->bool($r->hasHeader('X-Out-Third'))->is(true);
                return $r->withHeader('X-Out-Fourth', '1');
            },
            $runner->layer(),
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                $assert->mixed($a->getAttribute('bar'))->identicalTo('baz');
                $a = $a->withAttribute('baz', 'biz');
                $r = $c($a, $b);
                return $r->withHeader('X-Out-First', '1');
            }
        ];
        $runner2 = new Runner($queue2);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $runner2->run($request, $response);
        $assert->string($out->getHeaderLine('X-Out-Fourth'))->is("1");
    }
}
