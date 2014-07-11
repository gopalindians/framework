<?php
namespace Titon\Controller;

use Titon\Controller\Action\AbstractAction;
use Titon\Controller\Controller\AbstractController;
use Titon\Controller\Controller\ErrorController;
use Titon\Http\Exception\NotFoundException;
use Titon\Http\Server\Request;
use Titon\Http\Server\Response;
use Titon\Test\TestCase;
use Titon\View\View\Engine\TemplateEngine;
use Titon\View\View\TemplateView;
use VirtualFileSystem\FileSystem;

/**
 * @property \Titon\Controller\Controller $object
 * @property \VirtualFileSystem\FileSystem $vfs
 */
class ControllerTest extends TestCase {

    protected function setUp() {
        parent::setUp();

        $this->vfs = new FileSystem();
        $this->vfs->createStructure([
            '/views/' => [
                'private/' => [
                    'errors/' => [
                        'error.tpl' => '<?php echo $message; ?>',
                        'http.tpl' => '<?php echo $code . \': \' . $message; ?>'
                    ],
                    'layouts/' => [
                        'default.tpl' => '<?php echo $this->getContent(); ?>'
                    ]
                ],
                'public/' => [
                    'core/' => [
                        'custom.tpl' => 'core:custom',
                        'index.tpl' => 'core:index'
                    ]
                ]
            ]
        ]);

        $this->object = new ControllerStub([
            'module' => 'module',
            'controller' => 'controller',
            'action' => 'action',
            'args' => [100, 25]
        ]);
        $this->object->setRequest(Request::createFromGlobals());
        $this->object->setResponse(new Response());
    }

    public function testDispatchAction() {
        $this->assertEquals('actionNoArgs', $this->object->dispatchAction('action-no-args'));
        $this->assertEquals('actionNoArgs', $this->object->dispatchAction('actionNoArgs'));
        $this->assertEquals('actionNoArgs', $this->object->dispatchAction('actionNoArgs', ['foo', 'bar']));
        $this->assertEquals(125, $this->object->dispatchAction('actionWithArgs'));
        $this->assertEquals(555, $this->object->dispatchAction('actionWithArgs', [505, 50]));
        $this->assertEquals(335, $this->object->dispatchAction('actionWithArgs', [335]));
        $this->assertEquals(0, $this->object->dispatchAction('actionWithArgs', ['foo', 'bar']));
    }

    /**
     * @expectedException \Titon\Controller\Exception\InvalidActionException
     */
    public function testDispatchActionNullAction() {
        $this->object->dispatchAction(null);
    }

    /**
     * @expectedException \Titon\Controller\Exception\InvalidActionException
     */
    public function testDispatchActionMissingAction() {
        $this->object->dispatchAction('noAction');
    }

    /**
     * @expectedException \Titon\Controller\Exception\InvalidActionException
     */
    public function testDispatchActionPrivateAction() {
        $this->object->dispatchAction('actionPrivate');
    }

    /**
     * @expectedException \Titon\Controller\Exception\InvalidActionException
     */
    public function testDispatchActionInheritedAction() {
        $this->object->dispatchAction('dispatchAction');
    }

    public function testForwardAction() {
        $this->object->forwardAction('actionNoArgs');
        $this->assertEquals('actionNoArgs', $this->object->getConfig('action'));

        $this->object->forwardAction('actionWithArgs');
        $this->assertEquals('actionWithArgs', $this->object->getConfig('action'));
    }

    public function testRendering() {
        $view = new TemplateView($this->vfs->path('/views/'));
        $view->setEngine(new TemplateEngine());

        $controller = new ErrorController(['module' => 'main', 'controller' => 'core', 'action' => 'index']);
        $controller->setRequest(Request::createFromGlobals());
        $controller->setResponse(new Response());
        $controller->setView($view);
        $controller->initialize();

        // Using config
        $this->assertEquals('core:index', $controller->renderView());

        // Custom template
        $this->assertEquals('core:custom', $controller->renderView('core\custom'));

        // Custom template config
        $controller->setConfig('template', 'core/custom');
        $this->assertEquals('core:custom', $controller->renderView());
        $controller->setConfig('template', '');

        // Disable rendering
        $controller->setConfig('render', false);
        $this->assertEquals(null, $controller->renderView());

        // Error rendering
        $this->assertEquals('Message', $controller->renderError(new \Exception('Message')));
        $this->assertEquals(500, $controller->getResponse()->getStatusCode());

        // Turn off errors
        error_reporting(0);

        $this->assertEquals('404: Not Found', $controller->renderError(new NotFoundException('Not Found')));
        $this->assertEquals(404, $controller->getResponse()->getStatusCode());
    }

    public function testGetSetView() {
        $view = new TemplateView();
        $this->assertEquals(null, $this->object->getView());

        $this->object->setView($view);
        $this->assertEquals($view, $this->object->getView());
    }

}

class ControllerStub extends AbstractController {

    public function actionWithArgs($arg1, $arg2 = 0) {
        return $arg1 + $arg2;
    }

    public function actionNoArgs() {
        return 'actionNoArgs';
    }

    public function _actionPseudoPrivate() {
        return 'wontBeCalled';
    }

    protected function actionProtected() {
        return 'wontBeCalled';
    }

    private function actionPrivate() {
        return 'wontBeCalled';
    }

    public function renderView($template = null) { }

}